#include "cl_main_object.h"

#include <CL/cl.h>

////////////////////////////////////////

using namespace cl_;

namespace cl_{  /** begin namespace */

class clMainPrivate
{
public:
    clMainPrivate();
    ~clMainPrivate();

    bool init(int type);

    cl_context context = nullptr;
    cl_platform_id platform = nullptr;
    cl_uint num_platforms = 1;
    cl_device_id device_id = nullptr;
    cl_uint num_devices = 1;
    cl_command_queue command_queue = nullptr;
};

clMainPrivate::clMainPrivate()
{
}

clMainPrivate::~clMainPrivate()
{
    clReleaseDevice(device_id);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
}

bool clMainPrivate::init(int type)
{
    cl_int ret;

    cl_device_type dev = CL_DEVICE_TYPE_DEFAULT;

    if(type == clMainObject::GPU)
        dev = CL_DEVICE_TYPE_GPU;
    if(type == clMainObject::CPU)
        dev = CL_DEVICE_TYPE_CPU;

    ret = clGetPlatformIDs(1, &platform, &num_platforms);
    ret = clGetDeviceIDs(platform, dev, 1, &device_id, &num_devices);
    context = clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, &ret);
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

    return ret == 0;
}

/////////////////////

class clProgramPrivate{
public:
    cl_program program = nullptr;
    cl_kernel kernel = nullptr;
    cl_context context = nullptr;

    struct Mem{
        cl_mem memobj = nullptr;
        size_t size = 0;

        Mem(){}
        Mem(cl_context context, size_t size, int type){
            create(context, size, type);
        }

        ~Mem(){
            if(memobj){
                clReleaseMemObject(memobj);
            }
        }

        bool create(cl_context context, size_t len, int type){
            size = len;
            cl_int ret;

            cl_map_flags flags = CL_MEM_READ_WRITE;

            switch (type) {
            case clProgram::READ:
                flags = CL_MEM_READ_ONLY;
            break;
            case clProgram::WRITE:
                flags = CL_MEM_WRITE_ONLY;
            break;
            case clProgram::READWRITE:
                flags = CL_MEM_READ_WRITE;
            break;
            }

            memobj = clCreateBuffer(context, flags, len, nullptr, &ret);
            return ret == 0;
        }
    };

    typedef std::shared_ptr< Mem > PMem;

    std::vector<PMem> m_memobjs;

    clProgramPrivate(cl_context context){
        this->context = context;
    }

    ~clProgramPrivate(){

        if(kernel)
            clReleaseKernel(kernel);
        if(program)
            clReleaseProgram(program);
    }

    bool load(const std::string& source){
        cl_int ret;

        const char *c_cmd[] = {source.c_str()};
        size_t len[] = {source.length()};
        program = clCreateProgramWithSource(context, 1, c_cmd, len, &ret);
        return ret == 0;
    }

    bool createKernel(const std::string& kern){
        cl_int ret;
        kernel = clCreateKernel(program, kern.c_str(), &ret);
        return ret == 0;
    }

    clBuffer createBuffer(size_t size, int type){
        m_memobjs.push_back(std::make_shared<Mem>(context, size, type));
        return (clBuffer)(m_memobjs.size() - 1);
    }

    bool setArg(u_int32_t index, clBuffer buffer){
        PMem mem = m_memobjs[buffer];
        return 0 == clSetKernelArg(kernel, index, sizeof(cl_mem), mem->memobj);
    }
    bool setArg(u_int32_t index, int value){
        return 0 == clSetKernelArg(kernel, index, sizeof(int), &value);
    }
    bool setArg(u_int32_t index, float value){
        return 0 == clSetKernelArg(kernel, index, sizeof(float), &value);
    }
    bool setArg(u_int32_t index, double value){
        return 0 == clSetKernelArg(kernel, index, sizeof(double), &value);
    }

    bool write(cl_command_queue command_queue, clBuffer buffer, const bytevector &data){
        cl_int ret;

        PMem mem = m_memobjs[buffer];

        ret = clEnqueueWriteBuffer(command_queue, mem->memobj, CL_TRUE, 0, mem->size,
                                   data.data(), 0, nullptr, nullptr);
        return ret == 0;
    }

    bool read(cl_command_queue command_queue, clBuffer buffer, bytevector &data){
        cl_int ret;

        PMem mem = m_memobjs[buffer];

        data.resize(mem->size);

        ret = clEnqueueReadBuffer(command_queue, mem->memobj, CL_TRUE, 0, mem->size,
                                   data.data(), 0, nullptr, nullptr);
        return ret == 0;
    }

};

}/** end namespace */

////////////////////////////////////////

clMainObject clMainObject::m_instance;

clMainObject::clMainObject()
{
    m_priv = new clMainPrivate();
}

clMainObject::~clMainObject()
{
    delete m_priv;
}

bool clMainObject::init(int type)
{
    return m_priv->init(type);
}

std::shared_ptr<clProgram> clMainObject::getProgram(const std::string source)
{
    std::shared_ptr<clProgram> prog;
    prog.reset(new clProgram((clContext)m_priv->context));

    prog->m_priv->load(source);

    return prog;
}

clMainObject &clMainObject::instance()
{
    return m_instance;
}

//////////////////////

clProgram::clProgram(long long context)
{
    m_priv = new clProgramPrivate((cl_context)context);
}

clProgram::~clProgram()
{
    delete m_priv;
}

bool clProgram::createKernel(const std::string &kern)
{
    return m_priv->createKernel(kern);
}

clBuffer clProgram::createBuffer(size_t size, int type)
{
    return m_priv->createBuffer(size, type);
}

bool clProgram::write(clBuffer buffer, const bytevector &data)
{
    cl_command_queue cq = clMainObject::instance().m_priv->command_queue;
    return m_priv->write(cq, buffer, data);
}

bool clProgram::read(clBuffer buffer, bytevector &data)
{
    cl_command_queue cq = clMainObject::instance().m_priv->command_queue;
    return m_priv->read(cq, buffer, data);
}

bool clProgram::setArg(u_int32_t index, clBuffer buffer)
{
    return m_priv->setArg(index, buffer);
}

bool clProgram::setArg(u_int32_t index, int value)
{
    return m_priv->setArg(index, value);
}

bool clProgram::setArg(u_int32_t index, float value)
{
    return m_priv->setArg(index, value);
}

bool clProgram::setArg(u_int32_t index, double value)
{
    return m_priv->setArg(index, value);
}

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

    bool run(cl_kernel kernel, uint32_t work_dim, size_t works_count[]);

    cl_context context = nullptr;
    cl_platform_id platform = nullptr;
    cl_uint num_platforms = 1;
    cl_device_id device_id = nullptr;
    cl_uint num_devices = 1;
    cl_command_queue command_queue = nullptr;

    bool m_isInit;
};

clMainPrivate::clMainPrivate()
{
    m_isInit = false;
}

clMainPrivate::~clMainPrivate()
{
    clReleaseDevice(device_id);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
}

bool clMainPrivate::init(int type)
{
    if(m_isInit)
        return true;

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

    m_isInit = ret == 0;

    return ret == 0;
}

bool clMainPrivate::run(cl_kernel kernel, uint32_t work_dim, size_t works_count[])
{
    cl_int ret;
    ret = clEnqueueNDRangeKernel(command_queue, kernel, work_dim, nullptr, works_count,
                                     nullptr, 0, nullptr, nullptr);
    return ret == 0;
}

/////////////////////

class clProgramPrivate{
public:
    cl_program program = nullptr;
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
    std::vector<cl_kernel> m_kernels;

    clProgramPrivate(cl_context context){
        this->context = context;
    }

    ~clProgramPrivate(){

        for(cl_kernel kernel: m_kernels){
            clReleaseKernel(kernel);
        }
        if(program)
            clReleaseProgram(program);
    }

    bool build(cl_device_id device_id){
        cl_int ret;
        ret = clBuildProgram(program, 1, &device_id, nullptr, nullptr, nullptr);
        return ret == 0;
    }

    bool load(const std::string& source){
        cl_int ret;

        const char *c_cmd[] = {source.c_str()};
        size_t len[] = {source.length()};
        program = clCreateProgramWithSource(context, 1, c_cmd, len, &ret);
        return ret == 0;
    }

    clKernel createKernel(const std::string& kern){
        cl_int ret;
        cl_kernel kernel = clCreateKernel(program, kern.c_str(), &ret);
        m_kernels.push_back(kernel);
        return m_kernels.size() - 1;
    }
    cl_kernel get(clKernel val){
        return m_kernels[val];
    }

    clBuffer createBuffer(size_t size, int type){
        m_memobjs.push_back(std::make_shared<Mem>(context, size, type));
        return (clBuffer)(m_memobjs.size() - 1);
    }

    void freeBuffers(){
        m_memobjs.clear();
    }

    bool setArg(clKernel k, u_int32_t index, clBuffer buffer){
        cl_int ret;
        PMem mem = m_memobjs[buffer];
        cl_kernel kernel = m_kernels[k];
        ret = clSetKernelArg(kernel, index, sizeof(cl_mem), &mem->memobj);
        return ret == 0;
    }
    bool setArg(clKernel k, u_int32_t index, int value){
        cl_int ret;
        cl_kernel kernel = m_kernels[k];
        ret = clSetKernelArg(kernel, index, sizeof(int), &value);
        return ret == 0;
    }
    bool setArg(clKernel k, u_int32_t index, float value){
        cl_int ret;
        cl_kernel kernel = m_kernels[k];
        ret = clSetKernelArg(kernel, index, sizeof(float), &value);
        return ret == 0;
    }
    bool setArg(clKernel k, u_int32_t index, double value){
        cl_int ret;
        cl_kernel kernel = m_kernels[k];
        ret = clSetKernelArg(kernel, index, sizeof(double), &value);
        return ret == 0;
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

    bool write(cl_command_queue command_queue, clBuffer buffer, void *data){
        cl_int ret;

        PMem mem = m_memobjs[buffer];

        ret = clEnqueueWriteBuffer(command_queue, mem->memobj, CL_TRUE, 0, mem->size,
                                   data, 0, nullptr, nullptr);
        return ret == 0;
    }

    bool read(cl_command_queue command_queue, clBuffer buffer, void *data){
        cl_int ret;

        PMem mem = m_memobjs[buffer];

        ret = clEnqueueReadBuffer(command_queue, mem->memobj, CL_TRUE, 0, mem->size,
                                   data, 0, nullptr, nullptr);
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

bool clMainObject::run(clKernel kernel, const _clProgram &program, uint32_t work_dim, size_t works_count[])
{

    return m_priv->run(program->m_priv->get(kernel), work_dim, works_count);
}

bool clMainObject::run(clKernel kernel, const _clProgram &program, size_t works_count)
{
    size_t wc[] = {works_count};
    return m_priv->run(program->m_priv->get(kernel), 1, wc);
}

std::shared_ptr<clProgram> clMainObject::getProgram(const std::string source)
{
    std::shared_ptr<clProgram> prog;
    prog.reset(new clProgram((clContext)m_priv->context));

    prog->m_priv->load(source);

    return prog;
}

bool clMainObject::buildProgram(_clProgram prog)
{
    return prog->m_priv->build(m_priv->device_id);
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

clKernel clProgram::createKernel(const std::string &kern)
{
    return m_priv->createKernel(kern);
}

clBuffer clProgram::createBuffer(size_t size, int type)
{
    return m_priv->createBuffer(size, type);
}

void clProgram::freeBuffers()
{
    m_priv->freeBuffers();
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

bool clProgram::write(clBuffer buffer, void *data)
{
    cl_command_queue cq = clMainObject::instance().m_priv->command_queue;
    return m_priv->write(cq, buffer, data);

}

bool clProgram::read(clBuffer buffer, void *data)
{
    cl_command_queue cq = clMainObject::instance().m_priv->command_queue;
    return m_priv->read(cq, buffer, data);
}

bool clProgram::setArg(clKernel kernel, u_int32_t index, clBuffer buffer)
{
    return m_priv->setArg(kernel, index, buffer);
}

bool clProgram::setArg(clKernel kernel, u_int32_t index, int value)
{
    return m_priv->setArg(kernel, index, value);
}

bool clProgram::setArg(clKernel kernel, u_int32_t index, float value)
{
    return m_priv->setArg(kernel, index, value);
}

bool clProgram::setArg(clKernel kernel, u_int32_t index, double value)
{
    return m_priv->setArg(kernel, index, value);
}

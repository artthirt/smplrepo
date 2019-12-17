#include "cl_main_object.h"

#include <CL/cl.h>

////////////////////////////////////////

using namespace cl_;

namespace cl_{  /** begin namespace */

#ifdef _MSC_VER
#define CL_RUNTIME_EXPORT __declspec(dllexport)
#define CL	cl_

typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clBuildProgram_FN)(cl_program, cl_uint, const cl_device_id*, const char*, void (CL_CALLBACK*)
															 (cl_program, void*), void*);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clCompileProgram_FN)(cl_program, cl_uint, const cl_device_id*, const char*, cl_uint,
															   const cl_program*, const char**, void (CL_CALLBACK*) (cl_program, void*), void*);
typedef CL_RUNTIME_EXPORT cl_mem (CL_API_CALL*clCreateBuffer_FN)(cl_context, cl_mem_flags, size_t, void*, cl_int*);
typedef CL_RUNTIME_EXPORT cl_kernel (CL_API_CALL*clCreateKernel_FN)(cl_program, const char*, cl_int*);
typedef CL_RUNTIME_EXPORT cl_program (CL_API_CALL*clCreateProgramWithSource_FN)(cl_context, cl_uint, const char**, const size_t*, cl_int*);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clGetDeviceIDs_FN)(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clGetDeviceInfo_FN)(cl_device_id, cl_device_info, size_t, void*, size_t*);
typedef CL_RUNTIME_EXPORT cl_context (CL_API_CALL*clCreateContext_FN)(const cl_context_properties*, cl_uint, const cl_device_id*,
																   void (CL_CALLBACK*) (const char*, const void*, size_t, void*),
																   void*, cl_int*);
typedef CL_RUNTIME_EXPORT cl_context (CL_API_CALL*clCreateContext_FN)(const cl_context_properties*, cl_uint, const cl_device_id*, void (CL_CALLBACK*) (const char*, const void*, size_t, void*), void*, cl_int*);
typedef CL_RUNTIME_EXPORT cl_command_queue (CL_API_CALL*clCreateCommandQueue_FN)(cl_context, cl_device_id, cl_command_queue_properties, cl_int*);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clGetDeviceIDs_FN)(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clGetProgramBuildInfo_FN)(cl_program, cl_device_id, cl_program_build_info, size_t, void*, size_t*);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clEnqueueNDRangeKernel_FN)(cl_command_queue, cl_kernel, cl_uint, const size_t*, const size_t*, const size_t*, cl_uint, const cl_event*, cl_event*);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clEnqueueReadBuffer_FN)(cl_command_queue, cl_mem, cl_bool, size_t, size_t, void*, cl_uint, const cl_event*, cl_event*);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clEnqueueWriteBuffer_FN)(cl_command_queue, cl_mem, cl_bool, size_t, size_t, const void*, cl_uint, const cl_event*, cl_event*);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clFinish_FN)(cl_command_queue);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clFlush_FN)(cl_command_queue);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clGetPlatformIDs_FN)(cl_uint, cl_platform_id*, cl_uint*);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clGetPlatformInfo_FN)(cl_platform_id, cl_platform_info, size_t, void*, size_t*);
typedef CL_RUNTIME_EXPORT cl_program (CL_API_CALL*clLinkProgram_FN)(cl_context, cl_uint, const cl_device_id*, const char*, cl_uint, const cl_program*, void (CL_CALLBACK*) (cl_program, void*), void*, cl_int*);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clSetKernelArg_FN)(cl_kernel, cl_uint, size_t, const void*);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clReleaseCommandQueue_FN)(cl_command_queue);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clReleaseKernel_FN)(cl_kernel);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clReleaseMemObject_FN)(cl_mem);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clReleaseProgram_FN)(cl_program);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clReleaseContext_FN)(cl_context);
typedef CL_RUNTIME_EXPORT cl_int (CL_API_CALL*clReleaseDevice_FN)(cl_device_id);

clBuildProgram_FN				clBuildProgram           ;
clCompileProgram_FN				clCompileProgram         ;
clCreateBuffer_FN				clCreateBuffer           ;
clCreateKernel_FN				clCreateKernel           ;
clCreateProgramWithSource_FN	clCreateProgramWithSource;
clGetDeviceIDs_FN				clGetDeviceIDs           ;
clGetDeviceInfo_FN				clGetDeviceInfo          ;
clCreateContext_FN				clCreateContext          ;
clCreateCommandQueue_FN			clCreateCommandQueue     ;
clGetProgramBuildInfo_FN		clGetProgramBuildInfo    ;
clEnqueueNDRangeKernel_FN		clEnqueueNDRangeKernel   ;
clEnqueueReadBuffer_FN			clEnqueueReadBuffer      ;
clEnqueueWriteBuffer_FN			clEnqueueWriteBuffer     ;
clFinish_FN						clFinish                 ;
clFlush_FN						clFlush                  ;
clGetPlatformIDs_FN				clGetPlatformIDs         ;
clGetPlatformInfo_FN			clGetPlatformInfo        ;
clLinkProgram_FN				clLinkProgram            ;
clSetKernelArg_FN				clSetKernelArg           ;
clReleaseCommandQueue_FN		clReleaseCommandQueue    ;
clReleaseKernel_FN				clReleaseKernel          ;
clReleaseMemObject_FN			clReleaseMemObject       ;
clReleaseProgram_FN				clReleaseProgram         ;
clReleaseContext_FN				clReleaseContext         ;
clReleaseDevice_FN				clReleaseDevice          ;

#include <Windows.h>

class init_cl_functions{
public:

	init_cl_functions()
	{
		hMod = nullptr;
		cl_functions();
	}
	~init_cl_functions()
	{
		FreeLibrary(hMod);
	}

	void cl_functions()
	{
		hMod = LoadLibraryA("OpenCL.dll");

		if(!hMod){
			printf("opencl library not found\n");
			return;
		}

		clBuildProgram  = (clBuildProgram_FN)GetProcAddress(hMod, "clBuildProgram");
		clCompileProgram          = (clCompileProgram_FN         )	GetProcAddress(hMod, "clCompileProgram");
		clCreateBuffer            = (clCreateBuffer_FN           )	GetProcAddress(hMod, "clCreateBuffer");
		clCreateKernel            = (clCreateKernel_FN           )	GetProcAddress(hMod, "clCreateKernel");
		clCreateProgramWithSource = (clCreateProgramWithSource_FN)	GetProcAddress(hMod, "clCreateProgramWithSource");
		clGetDeviceIDs            = (clGetDeviceIDs_FN           )	GetProcAddress(hMod, "clGetDeviceIDs");
		clGetDeviceInfo           = (clGetDeviceInfo_FN          )	GetProcAddress(hMod, "clGetDeviceInfo");
		clCreateContext           = (clCreateContext_FN          )	GetProcAddress(hMod, "clCreateContext");
		clCreateCommandQueue      = (clCreateCommandQueue_FN     )	GetProcAddress(hMod, "clCreateCommandQueue");
		clGetProgramBuildInfo     = (clGetProgramBuildInfo_FN    )	GetProcAddress(hMod, "clGetProgramBuildInfo");
		clEnqueueNDRangeKernel    = (clEnqueueNDRangeKernel_FN   )	GetProcAddress(hMod, "clEnqueueNDRangeKernel");
		clEnqueueReadBuffer       = (clEnqueueReadBuffer_FN      )	GetProcAddress(hMod, "clEnqueueReadBuffer");
		clEnqueueWriteBuffer      = (clEnqueueWriteBuffer_FN     )	GetProcAddress(hMod, "clEnqueueWriteBuffer");
		clFinish                  = (clFinish_FN                 )	GetProcAddress(hMod, "clFinish");
		clFlush                   = (clFlush_FN                  )	GetProcAddress(hMod, "clFlush");
		clGetPlatformIDs          = (clGetPlatformIDs_FN         )	GetProcAddress(hMod, "clGetPlatformIDs");
		clGetPlatformInfo         = (clGetPlatformInfo_FN        )	GetProcAddress(hMod, "clGetPlatformInfo");
		clLinkProgram             = (clLinkProgram_FN            )	GetProcAddress(hMod, "clLinkProgram");
		clSetKernelArg            = (clSetKernelArg_FN           )	GetProcAddress(hMod, "clSetKernelArg");
		clReleaseCommandQueue     = (clReleaseCommandQueue_FN    )	GetProcAddress(hMod, "clReleaseCommandQueue");
		clReleaseKernel           = (clReleaseKernel_FN          )	GetProcAddress(hMod, "clReleaseKernel");
		clReleaseMemObject        = (clReleaseMemObject_FN       )	GetProcAddress(hMod, "clReleaseMemObject");
		clReleaseProgram          = (clReleaseProgram_FN         )	GetProcAddress(hMod, "clReleaseProgram");
		clReleaseContext          = (clReleaseContext_FN         )	GetProcAddress(hMod, "clReleaseContext");
		clReleaseDevice           = (clReleaseDevice_FN          )	GetProcAddress(hMod, "clReleaseDevice");
	}

private:
	HMODULE hMod;
};

init_cl_functions init;

#else
#define CL_RUNTIME_EXPORT
#define CL	cl
#endif

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

	ret = CL::clGetPlatformIDs(1, &platform, &num_platforms);
	ret = CL::clGetDeviceIDs(platform, dev, 1, &device_id, &num_devices);
	context = CL::clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, &ret);
	command_queue = CL::clCreateCommandQueue(context, device_id, 0, &ret);

    m_isInit = ret == 0;

    return ret == 0;
}

bool clMainPrivate::run(cl_kernel kernel, uint32_t work_dim, size_t works_count[])
{
    cl_int ret;
	ret = CL::clEnqueueNDRangeKernel(command_queue, kernel, work_dim, nullptr, works_count,
                                     nullptr, 0, nullptr, nullptr);

	clFlush(command_queue);
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
				CL::clReleaseMemObject(memobj);
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

			memobj = CL::clCreateBuffer(context, flags, len, nullptr, &ret);
            return ret == 0;
        }
    };

    typedef std::shared_ptr< Mem > PMem;

    std::vector<PMem> m_memobjs;
    std::vector<cl_kernel> m_kernels;

    std::string buildInfoString;

    clProgramPrivate(cl_context context){
        this->context = context;
    }

    ~clProgramPrivate(){

        for(cl_kernel kernel: m_kernels){
			CL::clReleaseKernel(kernel);
        }
        if(program)
			CL::clReleaseProgram(program);
    }

    bool build(cl_device_id device_id){
        cl_int ret;
		ret = CL::clBuildProgram(program, 1, &device_id, nullptr, nullptr, nullptr);

        size_t log_size = 0;

        if(ret != 0){
            clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);

            std::vector<char> data;
            data.resize(log_size + 1);
            data[data.size() - 1] = 0;
            clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, data.data(), nullptr);
			std::string tmp = data.data();
			buildInfoString = tmp;
        }

        return ret == 0;
    }

    bool load(const std::string& source){
        cl_int ret;

        const char *c_cmd[] = {source.c_str()};
        size_t len[] = {source.length()};
		program = CL::clCreateProgramWithSource(context, 1, c_cmd, len, &ret);
        return ret == 0;
    }

    clKernel createKernel(const std::string& kern){
        cl_int ret;
		cl_kernel kernel = CL::clCreateKernel(program, kern.c_str(), &ret);
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

	bool setArg(clKernel k, uint32_t index, clBuffer buffer){
        cl_int ret;
        PMem mem = m_memobjs[buffer];
        cl_kernel kernel = m_kernels[k];
		ret = CL::clSetKernelArg(kernel, index, sizeof(cl_mem), &mem->memobj);
        return ret == 0;
    }
	bool setArg(clKernel k, uint32_t index, int value){
        cl_int ret;
        cl_kernel kernel = m_kernels[k];
		ret = CL::clSetKernelArg(kernel, index, sizeof(int), &value);
        return ret == 0;
    }
	bool setArg(clKernel k, uint32_t index, float value){
        cl_int ret;
        cl_kernel kernel = m_kernels[k];
		ret = CL::clSetKernelArg(kernel, index, sizeof(float), &value);
        return ret == 0;
    }
	bool setArg(clKernel k, uint32_t index, double value){
        cl_int ret;
        cl_kernel kernel = m_kernels[k];
		ret = CL::clSetKernelArg(kernel, index, sizeof(double), &value);
        return ret == 0;
    }

    bool write(cl_command_queue command_queue, clBuffer buffer, const bytevector &data){
        cl_int ret;

        PMem mem = m_memobjs[buffer];

		ret = CL::clEnqueueWriteBuffer(command_queue, mem->memobj, CL_TRUE, 0, mem->size,
                                   data.data(), 0, nullptr, nullptr);
        return ret == 0;
    }

    bool read(cl_command_queue command_queue, clBuffer buffer, bytevector &data){
        cl_int ret;

        PMem mem = m_memobjs[buffer];

        data.resize(mem->size);

		ret = CL::clEnqueueReadBuffer(command_queue, mem->memobj, CL_TRUE, 0, mem->size,
                                   data.data(), 0, nullptr, nullptr);
        return ret == 0;
    }

    bool write(cl_command_queue command_queue, clBuffer buffer, void *data){
        cl_int ret;

        PMem mem = m_memobjs[buffer];

		ret = CL::clEnqueueWriteBuffer(command_queue, mem->memobj, CL_TRUE, 0, mem->size,
                                   data, 0, nullptr, nullptr);
        return ret == 0;
    }

    bool read(cl_command_queue command_queue, clBuffer buffer, void *data){
        cl_int ret;

        PMem mem = m_memobjs[buffer];

		ret = CL::clEnqueueReadBuffer(command_queue, mem->memobj, CL_TRUE, 0, mem->size,
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

std::string clProgram::getBuildInfoString() const
{
    return m_priv->buildInfoString;
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

bool clProgram::setArg(clKernel kernel, uint32_t index, clBuffer buffer)
{
    return m_priv->setArg(kernel, index, buffer);
}

bool clProgram::setArg(clKernel kernel, uint32_t index, int value)
{
    return m_priv->setArg(kernel, index, value);
}

bool clProgram::setArg(clKernel kernel, uint32_t index, float value)
{
    return m_priv->setArg(kernel, index, value);
}

bool clProgram::setArg(clKernel kernel, uint32_t index, double value)
{
    return m_priv->setArg(kernel, index, value);
}

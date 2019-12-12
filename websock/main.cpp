#include "mainwindow.h"

#include <QApplication>
#include <QFile>

#include "websock.h"

//////////////////////////////

#ifdef USE_OPENCL

#include <CL/cl.h>
#include "cl_main_object.h"

#endif

void testFun()
{
    Image im;
    loadImage("test.image", &im);
	PImage qim = createImage(&im);
	if(!qim.get() || qim->isNull())
        return;
	qim->save("test.bmp");

    std::string cmd_str =
            "__kernel void test(__global int* message, int w, int h)\n"
            "{\n"
            "   int gid = get_global_id(0);\n"
            "   int x = gid % w;\n"
            "   int y = gid / w;\n"
            "   message[gid] = x;\n"
            "}\n";

    ////////////////////////////

#ifdef USE_OPENCL

#if 0

    cl_int ret;
    cl_platform_id platform;
    cl_uint num_platforms = 1;
    cl_device_id device_id;
    cl_uint num_devices = 1;

    ret = clGetPlatformIDs(1, &platform, &num_platforms);

    ret = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device_id, &num_devices);

    cl_context context = clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, &ret);

    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

    cl_program program = nullptr;
    cl_kernel kernel = nullptr;

    const char *c_cmd = cmd_str.c_str();
    size_t len = cmd_str.length();

    program = clCreateProgramWithSource(context, 1, &c_cmd, &len, &ret);

    ret = clBuildProgram(program, 1, &device_id, nullptr, nullptr, nullptr);


    kernel = clCreateKernel(program, "test", &ret);

    cl_mem memobj = nullptr;
    const int memLenght = 100 * 50;
    cl_int mem[memLenght] = {0};

    memobj = clCreateBuffer(context, CL_MEM_READ_WRITE, memLenght * sizeof(cl_int), nullptr, &ret);

    ret = clEnqueueWriteBuffer(command_queue, memobj, CL_TRUE, 0, memLenght * sizeof(cl_int),
                               mem, 0, nullptr, nullptr);

    int w = 100, h = 50;

    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memobj);
    ret = clSetKernelArg(kernel, 1, sizeof(int), &w);
    ret = clSetKernelArg(kernel, 2, sizeof(int), &h);

    size_t global_work_size[1] = {100 * 50};

    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, nullptr, global_work_size,
                                 nullptr, 0, nullptr, nullptr);

    ret = clEnqueueReadBuffer(command_queue, memobj, CL_TRUE, 0, memLenght * sizeof(cl_int),
                              mem, 0, nullptr, nullptr);

    clReleaseMemObject(memobj);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseDevice(device_id);
    clReleaseCommandQueue(command_queue);

    clReleaseContext(context);

#endif

    bool ret = false;

    ret = cl_::clMainObject::instance().init();
    cl_::_clProgram prog = cl_::clMainObject::instance().getProgram(cmd_str);
    ret = cl_::clMainObject::instance().buildProgram(prog);

    const int memLenght = 100 * 50;
    cl_int mem[memLenght] = {0};

    cl_::clKernel kernel = prog->createKernel("test");
    cl_::clBuffer buf = prog->createBuffer(memLenght);
    ret = prog->write(buf, mem);

    ret = prog->setArg(kernel, 0, buf);
    ret = prog->setArg(kernel, 1, 100);
    ret = prog->setArg(kernel, 2, 50);

    ret = cl_::clMainObject::instance().run(kernel, prog, 50);

    ret = prog->read(buf, mem);

    printf("end test\n");
    //    std::vector< cl::Platform > platforms;

    //    cl::Platform::get(&platforms);

    //    if(platforms.empty())
    //        return;


    //    cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0};

    //    cl::Context context(CL_DEVICE_TYPE_GPU, properties);

    //    std::vector< cl::Device > devices = context.getInfo<CL_CONTEXT_DEVICES>();

    //    cl::Program::Sources source(1, std::make_pair(cmd_str.c_str(), cmd_str.length()));
    //    cl::Program program = cl::Program(context, source);

#endif
}

//////////////////////////////

int main(int argc, char *argv[])
{
	//testFun();

	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}

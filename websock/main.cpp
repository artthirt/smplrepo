#include "mainwindow.h"

#include <QApplication>
#include <QFile>

#include "websock.h"

//////////////////////////////

#ifdef USE_OPENCL

#include <CL/cl.h>
#include "cl_main_object.h"

class ConvertImage{
public:
    ConvertImage(){
        cl_::clMainObject::instance().init();

        m_progname = ":/cl/convert.cl";

        QString str;
        QFile f(QString::fromStdString(m_progname));
        if(f.open(QIODevice::ReadOnly)){
            str = f.readAll();
            f.close();
        }


        m_program = cl_::clMainObject::instance().getProgram(str.toStdString());
        bool res = cl_::clMainObject::instance().buildProgram(m_program);

        m_kernel = m_program->createKernel("convert");

    }

    QImage createImage(Image *picture){
        if(picture->width != m_image.width || picture->height != m_image.height){
            m_program->freeBuffers();
            m_image.width = picture->width;
            m_image.height = picture->height;

            m_output = QImage(picture->width, picture->height, QImage::Format_ARGB32);

            size_t Ysize = picture->linesize[0] * picture->height;
            size_t Usize = picture->linesize[1] * picture->height;
            size_t Vsize = picture->linesize[2] * picture->height;

            size_t RGBsize = picture->width * picture->height * 4;

            m_Y = m_program->createBuffer(Ysize, cl_::clProgram::WRITE);
            m_U = m_program->createBuffer(Usize, cl_::clProgram::WRITE);
            m_V = m_program->createBuffer(Vsize, cl_::clProgram::WRITE);
            m_Rgb = m_program->createBuffer(RGBsize, cl_::clProgram::READ);
        }

        m_program->write(m_Y, picture->data[0].data());
        m_program->write(m_U, picture->data[1].data());
        m_program->write(m_V, picture->data[2].data());

        m_program->setArg(m_kernel, 0, m_Y);
        m_program->setArg(m_kernel, 1, m_U);
        m_program->setArg(m_kernel, 2, m_V);
        m_program->setArg(m_kernel, 3, m_Rgb);
        m_program->setArg(m_kernel, 4, picture->linesize[0]);
        m_program->setArg(m_kernel, 5, picture->linesize[1]);
        m_program->setArg(m_kernel, 6, picture->linesize[2]);
        m_program->setArg(m_kernel, 7, picture->width * 4);
        m_program->setArg(m_kernel, 8, picture->width);
        m_program->setArg(m_kernel, 9, picture->height);

        cl_::clMainObject::instance().run(m_kernel, m_program, picture->width * picture->height);

        m_program->read(m_Rgb, m_output.bits());

        return m_output;
    }

private:
    cl_::_clProgram m_program;
    cl_::clKernel m_kernel;
    cl_::clBuffer m_Y;
    cl_::clBuffer m_U;
    cl_::clBuffer m_V;
    cl_::clBuffer m_Rgb;

    Image m_image;
    QImage m_output;

    std::string m_progname;
};

#endif

void testFun()
{
    Image im;
    loadImage("test.image", &im);
    QImage qim = createImage(&im);
    if(qim.isNull())
        return;
    qim.save("test.bmp");

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

    ConvertImage clImage;

    QImage im2 = clImage.createImage(&im);
    im2.save("test2.bmp");

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
    testFun();

	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}

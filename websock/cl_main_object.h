#ifndef CL_MAIN_OBJECT_H
#define CL_MAIN_OBJECT_H

#include <vector>
#include <string>

#include <memory>

typedef std::vector<uint8_t> bytevector;

namespace cl_{

class clMainObject;
class clMainPrivate;
class clProgramPrivate;

typedef long long clContext;

typedef unsigned long clBuffer;
typedef unsigned long clKernel;

class clProgram{
public:

    enum {READ, WRITE, READWRITE};

    clProgram(clContext context);
    ~clProgram();

    clKernel createKernel(const std::string& kern);
    /**
     * @brief createBuffer
     * @param size
     * @return index of buffer
     */
    clBuffer createBuffer(size_t size, int type = READWRITE);

    void freeBuffers();

    bool write(clBuffer buffer, const bytevector& data);
    bool read(clBuffer buffer, bytevector& data);

    bool write(clBuffer buffer, void *data);
    bool read(clBuffer buffer, void *data);

    bool setArg(clKernel kernel, u_int32_t index, clBuffer buffer);
    bool setArg(clKernel kernel, u_int32_t index, int value);
    bool setArg(clKernel kernel, u_int32_t index, float value);
    bool setArg(clKernel kernel, u_int32_t index, double value);

    friend class clMainObject;

private:
    clProgramPrivate* m_priv;
};

typedef std::shared_ptr<clProgram> _clProgram;

class clMainObject
{
public:
    enum {
        DEFAULT, GPU, CPU,
    };

    clMainObject();
    ~clMainObject();

    bool init(int type = GPU);

    bool run(clKernel kernel, const _clProgram& program, uint32_t work_dim, size_t works_count[]);
    bool run(clKernel kernel, const _clProgram& program, size_t works_count);

    std::shared_ptr<clProgram> getProgram(const std::string source);

    bool buildProgram(_clProgram prog);

    friend class clProgram;

    static clMainObject &instance();
private:
    clMainPrivate *m_priv;

    static clMainObject m_instance;
};

}

#endif // CL_MAIN_OBJECT_H

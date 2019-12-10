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

class clProgram{
public:

    clProgram(clContext context);
    ~clProgram();

    bool createKernel(const std::string& kern);
    /**
     * @brief createBuffer
     * @param size
     * @return index of buffer
     */
    clBuffer createBuffer(size_t size);

    bool write(clBuffer buffer, const bytevector& data);
    bool read(clBuffer buffer, bytevector& data);

    bool setArg(u_int32_t index, clBuffer buffer);
    bool setArg(u_int32_t index, int value);
    bool setArg(u_int32_t index, float value);
    bool setArg(u_int32_t index, double value);

    friend class clMainObject;

private:
    clProgramPrivate* m_priv;
};

class clMainObject
{
public:
    enum {
        DEFAULT, GPU, CPU,
    };

    clMainObject();
    ~clMainObject();

    bool init(int type);

    std::shared_ptr<clProgram> getProgram(const std::string source);

    friend class clProgram;

    static clMainObject &instance();
private:
    clMainPrivate *m_priv;

    static clMainObject m_instance;
};

}

#endif // CL_MAIN_OBJECT_H

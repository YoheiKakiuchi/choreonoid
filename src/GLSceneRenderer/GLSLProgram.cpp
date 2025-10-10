#include "GLSLProgram.h"
#include <QFile>
#include <cnoid/Format>
#include <stdexcept>
#include "gettext.h"

using namespace std;
using namespace cnoid;

#if 0
GLSLProgram::GLSLProgram()
{
    programHandle = 0;
    isLinked_ = false;
}
#endif

GLSLProgram::GLSLProgram(QOpenGLExtraFunctions* f)
{
    funcs = f;
    programHandle = 0;
    isLinked_ = false;
}


void GLSLProgram::release()
{
    if(programHandle){

        GLint numShaders = 0;
        funcs->glGetProgramiv(programHandle, GL_ATTACHED_SHADERS, &numShaders);

        if(numShaders > 0){
            vector<GLuint> shaderNames(numShaders);
            funcs->glGetAttachedShaders(programHandle, numShaders, NULL, &shaderNames.front());

            for(GLint i = 0; i < numShaders; i++){
                funcs->glDeleteShader(shaderNames[i]);
            }
        }
        
        funcs->glDeleteProgram(programHandle);
        programHandle = 0;
        isLinked_ = false;
    }
}


void GLSLProgram::loadShader(const char* filename, int shaderType)
{
    qDebug() << "f : " << filename;
    QFile file(filename);

    if(!file.exists()){
        qDebug() << "not found f: " << filename;
        throw std::runtime_error(formatR(_("Shader \"{}\" is not found."), filename));
    }
    
    file.open(QIODevice::ReadOnly);
    const QByteArray data = file.readAll();
    const GLchar* codes[] = { data.data() };
    const GLint codeSizes[] = { static_cast<int>(data.size()) };
    qDebug() << "c0 : " << codeSizes[0];
    GLuint shaderHandle = funcs->glCreateShader(shaderType);
    qDebug() << "c1 : " << shaderHandle;
    funcs->glShaderSource(shaderHandle, 1, codes, codeSizes);
    qDebug() << "c2";
    funcs->glCompileShader(shaderHandle);

    qDebug() << "c3";
    GLint result;
    funcs->glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &result);
    if(result == GL_FALSE){
        qDebug() << "c4";
        string msg;
        GLint length;
        funcs->glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length);
        if(length > 0){
            vector<char> log(length);
            GLsizei written;
            funcs->glGetShaderInfoLog(shaderHandle, length, &written, &log[0]);
            msg = formatR(_("Shader compilation of \"{0}\" failed.\n{1}"), filename, &log[0]);
        } else {
            msg = formatR(_("Shader compilation of \"{}\" failed."), filename);
        }
        qDebug() << msg;
        funcs->glDeleteShader(shaderHandle);
        throw std::runtime_error(msg);

    } else {
        if(!programHandle){
            programHandle = funcs->glCreateProgram();
            if(!programHandle){
                throw std::runtime_error(_("Unable to create shader program."));
            }
        }
        funcs->glAttachShader(programHandle, shaderHandle);
    }
}


void GLSLProgram::link()
{
    if(isLinked_){
        return;
    }
    
    if(!programHandle){
        qDebug() << "Program has not been compiled.";
        throw std::runtime_error(_("Program has not been compiled."));
    }

    funcs->glLinkProgram(programHandle);

    GLint status;
    funcs->glGetProgramiv(programHandle, GL_LINK_STATUS, &status);
    if(status == GL_FALSE){
        string msg;
        GLint length;
        funcs->glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &length);
        if(length > 0){
            vector<char> log(length);
            GLsizei written;
            funcs->glGetProgramInfoLog(programHandle, length, &written, &log[0]);
            msg = formatC("Program link failed:\n{}", &log[0]);
        } else {
            msg = _("Program link failed.");
        }
        qDebug() << msg;
        throw std::runtime_error(msg);
    }

    isLinked_ = true;
}


void GLSLProgram::validate()
{
    if(!programHandle || !isLinked_){
        throw std::runtime_error(_("Program is not linked"));
    }

    funcs->glValidateProgram(programHandle);

    GLint status;
    funcs->glGetProgramiv(programHandle, GL_VALIDATE_STATUS, &status);
    if(status == GL_FALSE){
        string msg;
        int length = 0;
        funcs->glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &length);
        if(length > 0){
            vector<char> log(length);
            GLsizei written;
            funcs->glGetProgramInfoLog(programHandle, length, &written, &log[0]);
            msg = formatC("Program failed to validate\n{}", &log[0]);
        } else {
            msg = _("Program failed to validate");
        }
        throw std::runtime_error(msg);
    }
}


void GLSLProgram::use()
{
    if(!programHandle || !isLinked_){
        throw std::runtime_error(_("Shader has not been linked."));
    }
    funcs->glUseProgram(programHandle);
}


GLSLUniformBlockBuffer::GLSLUniformBlockBuffer()
{
    uboHandle = 0;
    lastProgramHandle = 0;
}


GLSLUniformBlockBuffer::~GLSLUniformBlockBuffer()
{

}


bool GLSLUniformBlockBuffer::initialize(GLSLProgram& program, const std::string& blockName)
{
    this->blockName = blockName;
    
    funcs = program.functions(); // store funcs
    lastProgramHandle = program.handle();
    
    GLuint blockIndex = funcs->glGetUniformBlockIndex(program.handle(), blockName.c_str());
    if(blockIndex == GL_INVALID_INDEX){
        return false;
    }
    
    GLint blockSize;
    funcs->glGetActiveUniformBlockiv(program.handle(), blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
    localBuffer.resize(blockSize);

    funcs->glGenBuffers(1, &uboHandle);
    funcs->glBindBuffer(GL_UNIFORM_BUFFER, uboHandle);
    funcs->glBufferData(GL_UNIFORM_BUFFER, localBuffer.size(), NULL, GL_DYNAMIC_DRAW);

    return true;
}


GLuint GLSLUniformBlockBuffer::checkUniform(const char* name)
{
    GLuint index;
    funcs->glGetUniformIndices(lastProgramHandle, 1, &name, &index);

    if(index >= infos.size()){
        infos.resize(index + 1);
    }
    
    funcs->glGetActiveUniformsiv(lastProgramHandle, 1, &index, GL_UNIFORM_OFFSET, &(infos[index].offset));

    return index;
}


GLuint GLSLUniformBlockBuffer::checkUniformMatrix(const char* name)
{
    GLuint index = checkUniform(name);
    funcs->glGetActiveUniformsiv(lastProgramHandle, 1, &index, GL_UNIFORM_MATRIX_STRIDE, &(infos[index].matrixStrides));

    return index;
}

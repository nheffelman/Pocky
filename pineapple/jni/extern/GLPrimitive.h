#ifndef GLPRIMITIVE_H
#define GLPRIMITIVE_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "GLShaderProgram.h"
struct GLVertex
{
  Float3 p, n, t;
};


class GLPrimitive {
  public:
      ~GLPrimitive();

      virtual void tesselate(Float3 tess, Float3 translate, Float3 scale) = 0; //tesselates and reuploads into vbo
      void draw(GLShaderProgram *program);
      void draw(GLShaderProgram *program, int instances);

      GLuint vertexID() { return vertexId_; }
      GLuint indexID() { return indexId_; }

      const Float3& scale() { return scale_; }
      const Float3& translate() { return scale_; }
  protected:
      GLPrimitive(Float3 &tess, Float3 &translate, Float3 &scale);

      GLuint vertexId_, indexId_;
      GLenum type_;
      GLuint idxCount_;
      GLuint typeCount_;
      int vOffset_, tOffset_, nOffset_;

      Float3 scale_, translate_;
};

class GLQuad : public GLPrimitive {
    public:
	GLQuad(Float3 tess, Float3 translate, Float3 scale);
	~GLQuad();

	void tesselate(Float3 tess, Float3 translate, Float3 scale);
};

class GLRect : public GLPrimitive {
public:
    GLRect(Float3 tess, Float3 translate, Float3 scale);
    ~GLRect();

    void tesselate(Float3 tess, Float3 translate, Float3 scale);
};

class GLPlane : public GLPrimitive {
    public:
	GLPlane(Float3 tess, Float3 translate, Float3 scale);
	~GLPlane();

	void tesselate(Float3 tess, Float3 translate, Float3 scale);
};


#endif // GLPRIMITIVE_H
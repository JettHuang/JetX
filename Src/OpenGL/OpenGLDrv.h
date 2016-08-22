// \brief
//	abstract for opengl device layer. based on openg3.3 core
//

#ifndef __JETX_OPENGL_H__
#define __JETX_OPENGL_H__

#include <GL/glew.h>

class FOpenGLDrv
{
public:
	static FOpenGLDrv& SharedInstance();

	bool Initialize();
	void Terminate();

protected:
	FOpenGLDrv();
	virtual ~FOpenGLDrv();
};



#endif // __JETX_OPENGL_H__

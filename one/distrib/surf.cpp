#include "surf.h"
#include "extra.h"
using namespace std;

namespace
{
    
    // We're only implementing swept surfaces where the profile curve is
    // flat on the xy-plane.  This is a check function.
    static bool checkFlat(const Curve &profile)
    {
        for (unsigned i=0; i<profile.size(); i++)
            if (profile[i].V[2] != 0.0 ||
                profile[i].T[2] != 0.0 ||
                profile[i].N[2] != 0.0)
                return false;
    
        return true;
    }
}

bool vec_equals(Vector3f v1,Vector3f v2,float tol){
	return (abs(v1.x()-v2.x())<tol&&abs(v1.y()-v2.y())<tol&&abs(v1.z()-v2.z())<tol);
}

Surface makeSurfRev(const Curve &profile, unsigned steps)
{
    Surface surface;
    
    if (!checkFlat(profile))
    {
        cerr << "surfRev profile curve must be flat on xy plane." << endl;
        exit(0);
    }
	float angle=0;
    for(int j =0;j<=steps;j++){
	    for(int i =0;i<profile.size();i++){
			Matrix3f rota=Matrix3f::rotateY(angle);
			surface.VV.push_back(rota*profile[i].V);
			surface.VN.push_back(-1*(rota.inverse().transposed()*profile[i].N));
		}
		angle+=2*M_PI/steps;
	}
	for (int i =0;i<surface.VV.size()-profile.size()-1;i++){
		surface.VF.push_back(Tup3u(i,i+profile.size()+1,i+profile.size()));
		surface.VF.push_back(Tup3u(i+1,i+profile.size()+1,i));
	}

 
    return surface;
}

Surface makeGenCyl(const Curve &profile, const Curve &sweep )
{
    Surface surface;

    if (!checkFlat(profile))
    {
        cerr << "genCyl profile curve must be flat on xy plane." << endl;
        exit(0);
    }
	bool closed= vec_equals(sweep[0].V,sweep[sweep.size()-1].V,1e-5)&& vec_equals(sweep[0].T,sweep[sweep.size()-1].T,1e-5);
	float incn=-acos(Vector3f::dot(sweep[0].N.normalized(),sweep[sweep.size()-1].N.normalized()))/(sweep.size()-1);
	float rotn=0;

	for(int i=0;i<sweep.size();i++){	
        for(int j =0;j < profile.size();j++){
			Matrix4f M;
			M.setCol( 0, Vector4f( (Matrix3f::rotation(sweep[i].T,rotn)*sweep[i].N).normalized(), 0 ) );
			M.setCol( 1, Vector4f( (Matrix3f::rotation(sweep[i].T,rotn)*sweep[i].B).normalized(), 0 ) );
            M.setCol( 2, Vector4f( sweep[i].T, 0 ) );
            M.setCol( 3, Vector4f( sweep[i].V, 1 ) );
	
			Vector3f v1=(M*Vector4f(profile[j].V,1)).xyz();	
			Matrix3f MIT=M.getSubmatrix3x3(0,0).inverse().transposed();
	
			Vector3f n1=-1*(MIT*profile[j].N);
						 
			surface.VV.push_back(v1);
			surface.VN.push_back(n1);
			}
		if(closed){rotn+=incn;}
	}
	for (int i =0;i<surface.VV.size()-profile.size()-1;i++){
		surface.VF.push_back(Tup3u(i+profile.size()+1,i+profile.size(),i+1));
		surface.VF.push_back(Tup3u(i+1,i+profile.size(),i));
	}

    return surface;
}

void drawSurface(const Surface &surface, bool shaded)
{
    // Save current state of OpenGL
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if (shaded)
    {
        // This will use the current material color and light
        // positions.  Just set these in drawScene();
        glEnable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // This tells openGL to *not* draw backwards-facing triangles.
        // This is more efficient, and in addition it will help you
        // make sure that your triangles are drawn in the right order.
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    else
    {        
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        glColor4f(0.4f,0.4f,0.4f,1.f);
        glLineWidth(1);
    }

    glBegin(GL_TRIANGLES);
    for (unsigned i=0; i<surface.VF.size(); i++)
    {
        glNormal(surface.VN[surface.VF[i][0]]);
        glVertex(surface.VV[surface.VF[i][0]]);
        glNormal(surface.VN[surface.VF[i][1]]);
        glVertex(surface.VV[surface.VF[i][1]]);
        glNormal(surface.VN[surface.VF[i][2]]);
        glVertex(surface.VV[surface.VF[i][2]]);
    }
    glEnd();

    glPopAttrib();
}

void drawNormals(const Surface &surface, float len)
{
    // Save current state of OpenGL
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable(GL_LIGHTING);
    glColor4f(0,1,1,1);
    glLineWidth(1);

    glBegin(GL_LINES);
    for (unsigned i=0; i<surface.VV.size(); i++)
    {
        glVertex(surface.VV[i]);
        glVertex(surface.VV[i] + surface.VN[i] * len);
    }
    glEnd();

    glPopAttrib();
}

void outputObjFile(ostream &out, const Surface &surface)
{
    
    for (unsigned i=0; i<surface.VV.size(); i++)
        out << "v  "
            << surface.VV[i][0] << " "
            << surface.VV[i][1] << " "
            << surface.VV[i][2] << endl;

    for (unsigned i=0; i<surface.VN.size(); i++)
        out << "vn "
            << surface.VN[i][0] << " "
            << surface.VN[i][1] << " "
            << surface.VN[i][2] << endl;

    out << "vt  0 0 0" << endl;
    
    for (unsigned i=0; i<surface.VF.size(); i++)
    {
        out << "f  ";
        for (unsigned j=0; j<3; j++)
        {
            unsigned a = surface.VF[i][j]+1;
            out << a << "/" << "1" << "/" << a << " ";
        }
        out << endl;
    }
}

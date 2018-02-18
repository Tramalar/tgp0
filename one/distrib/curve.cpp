#include "curve.h"
#include "extra.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
using namespace std;

namespace


{
    // Approximately equal to.  We don't want to use == because of
    // precision issues with floating point.
    inline bool approx( const Vector3f& lhs, const Vector3f& rhs )
    {
        const float eps = 1e-8f;
        return ( lhs - rhs ).absSquared() < eps;
    }

    
}
Matrix4f Bez=Matrix4f(1,-3, 3,-1,
		              0, 3,-6, 3,
		              0, 0, 3,-3,
		              0, 0, 0, 1);
Matrix4f BS=Matrix4f (1.f/6,-3.f/6, 3.f/6,-1.f/6,
		              4.f/6, 0.f/6,-6.f/6, 3.f/6,
		              1.f/6, 3.f/6, 3.f/6,-3.f/6,
		              0.f/6, 0.f/6, 0.f/6, 1.f/6);

//tekee neljästä Vector3f:stä matriisin
//oletus on parametrina annetussa vektorissa tosiaan on neljä pistettä...
Matrix4f flatten(vector<Vector3f> controls){
	return Matrix4f(controls[0][0],controls[1][0],controls[2][0],controls[3][0],
				    controls[0][1],controls[1][1],controls[2][1],controls[3][1],
					controls[0][2],controls[1][2],controls[2][2],controls[3][2],
					0,0,0,0);
}
    
//Mahdollistin aikaisemman binormaalin antamisen parametrina, jotta evalBsplinessa voidaan kutsua tätä ja pysyy normaalit linjassa.
Curve evalBezier( const vector< Vector3f >& P, unsigned steps, Vector3f prevB )
{
    // Check
    if( P.size() < 4 || P.size() % 3 != 1 )
    {
        cerr << "evalBezier must be called with 3n+1 control points." << endl;
        exit( 0 );
    }

    // TODO:
    // You should implement this function so that it returns a Curve
    // (e.g., a vector< CurvePoint >).  The variable "steps" tells you
    // the number of points to generate on each piece of the spline.
    // At least, that's how the sample solution is implemented and how
    // the SWP files are written.  But you are free to interpret this
    // variable however you want, so long as you can control the
    // "resolution" of the discretized spline curve with it.

    // Make sure that this function computes all the appropriate
    // Vector3fs for each CurvePoint: V,T,N,B.
    // [NBT] should be unit and orthogonal.

    // Also note that you may assume that all Bezier curves that you
    // receive have G1 continuity.  Otherwise, the TNB will not be
    // be defined at points where this does not hold.

    cerr << "\t>>> evalBezier has been called with the following input:" << endl;

    cerr << "\t>>> Control points (type vector< Vector3f >): "<< endl;
	vector<Vector3f> controls=vector<Vector3f>();
	Curve points=Curve();
	CurvePoint cp;
	cp.B=prevB;
    for( unsigned i = 0; i < P.size(); ++i )
    {
		controls.push_back(P[i]);
		if(controls.size()==4){
			for(unsigned t=0;t<=steps;t++){
				float p=(t+0.f)/steps;

				Vector4f T=Vector4f(1,p,p*p,p*p*p);
				//kannan derivaatta
				Vector4f T_d=Vector4f(0,1,2*p,3*p*p);
				
				Matrix4f G=flatten(controls);

				cp.V=(G*Bez*T).xyz();

				cp.T=(G*Bez*T_d).xyz().normalized();

				cp.N=Vector3f::cross(cp.B,cp.T);

				cp.B=Vector3f::cross(cp.T,cp.N);

				points.push_back(cp);
			}
			controls=vector<Vector3f>();
			controls.push_back(P[i]);
		}
    }
    cerr << "\t>>> Steps (type steps): " << steps << endl;

    // Right now this will just return this empty curve.
    return points;
}

//kutsutaan kun pitää asettaa ensimmäinen binormaali
Curve evalBezier( const vector< Vector3f >& P, unsigned steps )
{
    return evalBezier(P,steps,Vector3f(0,0,1));
}


//toteutettu MIT:n ehdottamalla tavalla, evalBezieriä hyödyntäen
//Käydään kontrollipisteitä läpi neljän pisteen ikkunoissa, joille luodaan bezier-käyrät.
Curve evalBspline( const vector< Vector3f >& P, unsigned steps )
{
    // Check
    if( P.size() < 4 )
    {
        cerr << "evalBspline must be called with 4 or more control points." << endl;
        exit( 0 );
    }

    // TODO:
    // It is suggested that you implement this function by changing
    // basis from B-spline to Bezier.  That way, you can just call
    // your evalBezier function.

    cerr << "\t>>> evalBSpline has been called with the following input:" << endl;

    cerr << "\t>>> Control points (type vector< Vector3f >): "<< endl;
	vector<Vector3f> controls=vector<Vector3f>();
	Curve bez;
    for( unsigned i = 0; i < P.size(); ++i ){
		controls.push_back(P[i]);
		if(controls.size()==4){
        	vector<Vector3f> newP=vector<Vector3f>();
			Matrix4f G=flatten(controls);
			Matrix4f newcont=(G*BS*Bez.inverse());

			newP.push_back(Vector3f(newcont[0],newcont[1],newcont[2]));
			newP.push_back(Vector3f(newcont[4],newcont[5],newcont[6]));
			newP.push_back(Vector3f(newcont[8],newcont[9],newcont[10]));
			newP.push_back(Vector3f(newcont[12],newcont[13],newcont[14]));

			controls=vector<Vector3f>();
			controls.push_back(P[i-2]);
			controls.push_back(P[i-1]);
			controls.push_back(P[i]);
			Curve temp;
			//asetetaan ensimmäinen binormaali mikäli käyrään ei vielä olla lisätty pisteitä
			if(bez.size()>0)
				temp=evalBezier(newP,steps, bez[bez.size()-1].B);
			else
				temp=evalBezier(newP,steps);
		    for (unsigned k=0;k<temp.size();k++)
			    bez.push_back(temp[k]);
		}
    }

    cerr << "\t>>> Steps (type steps): " << steps << endl;

    return bez;
}

Curve evalCircle( float radius, unsigned steps )
{
    // This is a sample function on how to properly initialize a Curve
    // (which is a vector< CurvePoint >).
    
    // Preallocate a curve with steps+1 CurvePoints
    Curve R( steps+1 );

    // Fill it in counterclockwise
    for( unsigned i = 0; i <= steps; ++i )
    {
        // step from 0 to 2pi
        float t = 2.0f * M_PI * float( i ) / steps;

        // Initialize position
        // We're pivoting counterclockwise around the y-axis
        R[i].V = radius * Vector3f( cos(t), sin(t), 0 );
        
        // Tangent vector is first derivative
        R[i].T = Vector3f( -sin(t), cos(t), 0 );
        
        // Normal vector is second derivative
        R[i].N = Vector3f( -cos(t), -sin(t), 0 );

        // Finally, binormal is facing up.
        R[i].B = Vector3f( 0, 0, 1 );
    }

    return R;
}

//piirtää yleisiä torus-solmuja, esimerkki knot.swp-tiedostossa p=9, q=8
Curve evalKnot( unsigned steps, float radius,int p, int q)
{
    Curve R( steps+1 );

	Vector3f prevb(0,0,1);
    for( unsigned i = 0; i <= steps; ++i )
    {
        // step from 0 to 2pi
        float t = 2.0f * M_PI * float( i ) / steps;
		//Parametrinen yhtälö wikipediasta...
		R[i].V=radius*Vector3f((cos(q*t)+2)*cos(p*t),(cos(q*t)+2)*sin(p*t),-1*(sin(q*t)));
		//derivaatat wolfram alphalla
		R[i].T=Vector3f(-1*(p*sin(p*t)*(cos(q*t)+2))-q*(cos(p *t)*sin(q*t)),
			             p *cos(p* t)* (cos(q* t) + 2) - q* (sin(p* t)* sin(q* t)),
			            -1*(q*cos(q*t))).normalized();
		

		R[i].N=Vector3f::cross(prevb,R[i].T).normalized();

		prevb=R[i].B=Vector3f::cross(R[i].T,R[i].N).normalized();

    }

    return R;
}


void drawCurve( const Curve& curve, float framesize )
{
    // Save current state of OpenGL
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // Setup for line drawing
    glDisable( GL_LIGHTING ); 
    glColor4f( 1, 1, 1, 1 );
    glLineWidth( 1 );
    
    // Draw curve
    glBegin( GL_LINE_STRIP );
    for( unsigned i = 0; i < curve.size(); ++i )
    {
        glVertex( curve[ i ].V );
    }
    glEnd();

    glLineWidth( 1 );

    // Draw coordinate frames if framesize nonzero
    if( framesize != 0.0f )
    {
        Matrix4f M;

        for( unsigned i = 0; i < curve.size(); ++i )
        {
            M.setCol( 0, Vector4f( curve[i].N, 0 ) );
            M.setCol( 1, Vector4f( curve[i].B, 0 ) );
            M.setCol( 2, Vector4f( curve[i].T, 0 ) );
            M.setCol( 3, Vector4f( curve[i].V, 1 ) );

            glPushMatrix();
            glMultMatrixf( M );
            glScaled( framesize, framesize, framesize );
            glBegin( GL_LINES );
            glColor3f( 1, 0, 0 ); glVertex3d( 0, 0, 0 ); glVertex3d( 1, 0, 0 );
            glColor3f( 0, 1, 0 ); glVertex3d( 0, 0, 0 ); glVertex3d( 0, 1, 0 );
            glColor3f( 0, 0, 1 ); glVertex3d( 0, 0, 0 ); glVertex3d( 0, 0, 1 );
            glEnd();
            glPopMatrix();
        }
    }
    
    // Pop state
    glPopAttrib();
}


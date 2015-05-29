uniform mat4	ciModelView;
uniform mat4	ciModelViewProjection;
#if !defined( INSTANCED_MODEL )
uniform mat3	ciNormalMatrix;
#endif
in vec4			ciPosition;
in vec3 		ciNormal;
in vec4 		ciColor;

out Vertex
{
	vec3 color;
	vec3 normal;
	vec3 position;
} vertex;

#if defined( INSTANCED_MODEL )
in mat4			vInstanceModelMatrix;
in mat3			vInstanceNormalMatrix;
#endif

void main( void )
{
	vertex.color		= ciColor.rgb;

#if defined( INSTANCED_MODEL )
	mat3 normalMatrix	= vInstanceNormalMatrix;
#else
	mat3 normalMatrix	= ciNormalMatrix;
#endif
	vec3 n				= ciNormal;
	vertex.normal		= normalMatrix * n;

	vertex.position		= ( ciModelView * ciPosition ).xyz;

	mat4 m				= ciModelViewProjection;
	vec4 p				= ciPosition;
#if defined( INSTANCED_MODEL )
	m					= m * vInstanceModelMatrix;
#endif
	gl_Position			= m * p;
}

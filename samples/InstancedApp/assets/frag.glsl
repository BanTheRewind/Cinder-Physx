const vec3 kLightPosition	= vec3( 0.0 );
const float kShininess		= 100.0;
const float kNormalization	= ( kShininess + 8.0 ) / ( 3.141592653589 * 8.0 );
const vec3 Is				= vec3( 1.0 );

in Vertex
{
	vec3 color;
	vec3 normal;
	vec3 position;
} vertex;

layout (location = 0) out vec4 oColor;

void main( void )
{
	vec3 L		= normalize( kLightPosition - vertex.position );
	vec3 E		= normalize( -vertex.position );
	vec3 N		= normalize( vertex.normal );
	vec3 H		= normalize( L + E );

	float Kd	= max( dot( N, L ), 0.0 );
	vec3 Id		= vertex.color * Kd;

	float Ks	= pow( max( dot( N, H ), 0.0 ), kShininess );
	vec3 Is		= kNormalization * Is * Ks;

	oColor		= vec4( Id + Is, 1.0 );
}
 
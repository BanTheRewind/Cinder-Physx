#include "Model.h"

using namespace ci;

Model::Model()
: mModelMatrix( mat4( 1.0f ) ), mNormalMatrix( mat3( 1.0f ) )
{
}

Model::Model( const Model& rhs )
{
	*this = rhs;
}

Model& Model::operator=( const Model& rhs )
{
	mModelMatrix	= rhs.mModelMatrix;
	mNormalMatrix	= rhs.mNormalMatrix;
	return *this;
}

Model& Model::modelMatrix( const mat4& m )
{
	mModelMatrix = m;
	return *this;
}

Model& Model::normalMatrix( const mat3& m )
{
	mNormalMatrix = m;
	return *this;
}

const mat4& Model::getModelMatrix() const
{
	return mModelMatrix;
}

const mat3& Model::getNormalMatrix() const
{
	return mNormalMatrix;
}

void Model::setModelMatrix( const mat4& m )
{
	mModelMatrix = m;
}

void Model::setNormalMatrix( const mat3& m )
{
	mNormalMatrix = m;
}

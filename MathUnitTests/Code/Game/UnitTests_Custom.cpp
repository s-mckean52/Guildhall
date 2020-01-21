//-----------------------------------------------------------------------------------------------
// UnitTests_Custom.cpp
//
// YOU MAY CHANGE anything in this file.  Make sure your "RunTestSet()" commands at the bottom
//	of the file in RunTest_Custom() all pass "false" for their first argument (isGraded).
//
#include "Game/UnitTests_Custom.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/FloatRange.hpp"
// Include whatever you want here to test various engine classes (even non-math stuff, like Rgba)


//-----------------------------------------------------------------------------------------------
int TestSet_Custom_Dummy()
{
	Vec2 arrayOfPoints[ 4 ] = {};
	OBB2 boxToProject = OBB2( Vec2( 50.f, 50.f ), Vec2( 3.f, 3.f ) );
	boxToProject.GetCornerPositions( arrayOfPoints );
	
	Vec2 arrayOfPoints2[ 4 ] = {};
	OBB2 boxToProject2 = OBB2( Vec2( 50.f, 50.f ), Vec2( 10.f, 3.f ) );
	boxToProject2.GetCornerPositions( arrayOfPoints2 );

	GetRangeOnProjectedAxis( 4, arrayOfPoints, Vec2(), boxToProject.GetIBasisNormal() );
	GetRangeOnProjectedAxis( 4, arrayOfPoints2, Vec2(), boxToProject2.GetIBasisNormal() );
	GetRangeOnProjectedAxis( 4, arrayOfPoints2, Vec2(), boxToProject2.GetJBasisNormal() );

	OBB2 boxDoesNotOverlap			= OBB2( Vec2( 150.f, 150.f ), Vec2( 10.f, 3.f ) );
	OBB2 boxDoesNotOverlapRotated	= OBB2( Vec2( 150.f, 150.f ), Vec2( 10.f, 3.f ), 55.f );

	VerifyTestResult( DoOBBAndOBBOverlap2D( boxToProject, boxToProject2 ) == true, "Project Boxes did not overlap" );
	VerifyTestResult( DoOBBAndOBBOverlap2D( boxToProject, boxDoesNotOverlap ) == false, "Boxes should not have overlapped" );
	VerifyTestResult( DoOBBAndOBBOverlap2D( boxToProject2, boxDoesNotOverlapRotated ) == false, "Rotated Box should not have overlapped" );
	//VerifyTestResult( RoundDownToInt( a + b ) == -10, "RoundDownToInt() was incorrect for -10.f -> -10 (int)" );


	return 3; // Number of tests expected (equal to the # of times you call VerifyTestResult)
}


//-----------------------------------------------------------------------------------------------
int TestSet_Custom_Rgba()
{
	// Write your own tests here; each call to VerifyTestResult is considered a "test".

	return 0; // Number of tests expected (set equal to the # of times you call VerifyTestResult)
}


//-----------------------------------------------------------------------------------------------
void RunTests_Custom()
{
	// Always set first argument to "false" for Custom tests, so they don't interfere with grading
	RunTestSet( false, TestSet_Custom_Dummy,	"Custom dummy sample tests" );
	RunTestSet( false, TestSet_Custom_Rgba,		"Custom Rgba sample tests" );
}


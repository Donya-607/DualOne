#include "StorageForScene.h"

StorageForScene::StorageForScene() :
	timer(), wasRetried( false )
{}
StorageForScene::~StorageForScene() = default;

void StorageForScene::Reset()
{
	timer.Set( 99, 59, 59 );
}

void  StorageForScene::StoreTimer( Timer storeData )
{
	timer = storeData;
}
Timer StorageForScene::GetTimer() const
{
	return timer;
}

void StorageForScene::StoreRetryFlag( bool flag )
{
	wasRetried = flag;
}
bool StorageForScene::GetRetryFlag() const
{
	return wasRetried;
}

#include "StorageForScene.h"

StorageForScene::StorageForScene() :
	timer()
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

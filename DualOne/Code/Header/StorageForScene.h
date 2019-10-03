#pragma once

#include "Donya/Template.h"

#include "Timer.h"

class StorageForScene final : public Donya::Singleton<StorageForScene>
{
	friend class Donya::Singleton<StorageForScene>;
private:
	Timer timer;
private:
	StorageForScene();
public:
	~StorageForScene();
public:
	void Reset();

	void  StoreTimer( Timer storeData );
	Timer GetTimer() const;
};

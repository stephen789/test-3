#include "llviewerprecompiledheaders.h"

#include "os_circuits.h"
#include "lluictrlfactory.h"
#include "llfloater.h"
#include "llbutton.h"
#include "llcheckboxctrl.h"
#include "llscrolllistctrl.h"
#include "lllineeditor.h"
#include "llnotificationsutil.h"
#include "llfloaterchat.h"
#include "llchat.h"

#include "llagent.h"
#include "llworld.h"
#include "message.h"
#include "llregionhandle.h"
#include "llviewermenu.h"

// --------------------------------------------------------------------------

RecentRegionList::RecentRegionList()
:	LLSingleton<RecentRegionList>()
{
}

void RecentRegionList::addRegion(const std::string& name, const std::string& handle, const LLUUID& id)
{
	if(unique_regionid[id]) return;
	unique_regionid[id] = true;

	RegionEntry entry;

	entry.name=name;
	entry.handle=handle;
	entry.regionid=id;

	mRegionList.push_back(entry);
}

void RecentRegionList::requestList(OSCircuits* circuits)
{
	if(circuits)
	{
		// send the list of recent regions to the region circuits floater
		for(std::deque<RegionEntry>::iterator iter=mRegionList.begin();
			iter!=mRegionList.end();
			++iter)
		{
			RegionEntry entry=*iter;
			circuits->addRegionList(entry.name,entry.handle,entry.regionid);
		}
		/*
		for (RecentRegionList::region_list_t::const_iterator iter = getRegionList().begin();
		 iter != getRegionList().end(); ++iter)
		{
			LLViewerRegion* regionp = *iter;
			circuits->addRegionList(regionp->getName(), U64_to_str(regionp->getHandle()), regionp->getRegionID());
		}
		*/
	}
}

void RecentRegionList::notifyEnabled(LLViewerRegion* regionp)
{
	static LLCachedControl<bool> we_want_notifications(gSavedSettings, "OSNotifyEnabledSimulator");

	if (!regionp)
	{
		LL_WARNS() << "OSCircuits::notifyEnabled() region is invalid!" <<  LL_ENDL;
		return;
	}

	//add to our region list
	if(regionp->getRegionID().notNull())
	addRegion(regionp->getName(), U64_to_str(regionp->getHandle()), regionp->getRegionID());
	if(floater_visible("region circuits"))
	OSCircuits::getInstance()->update();

	if (!we_want_notifications) return;
	LLHost host = regionp->getHost();
	LLChat chat;
	chat.mText = llformat("Added region '%s' - Host:%s Colo:%s ClassID:%i cpuRatio:%i SKU:%s Type:%s",
		regionp->getName().c_str(),
		regionp->getHost().getIPandPort().c_str(),
		regionp->getSimColoName().c_str(),
		regionp->getSimClassID(),
		regionp->getSimCPURatio(),
		regionp->getSimProductSKU().c_str(),
		regionp->getLocalizedSimProductName().c_str());
	chat.mSourceType = CHAT_SOURCE_SYSTEM;
	LLFloaterChat::addChat(chat);
}

void RecentRegionList::notifyDisabled(LLViewerRegion* regionp)
{
	static LLCachedControl<bool> we_want_notifications(gSavedSettings, "OSNotifyDisabledSimulator");
	
	if (!regionp)
	{
		LL_WARNS() << "OSCircuits::notifyDisabled() region is invalid!" <<  LL_ENDL;
		return;
	}

	//add to our region list
	if(regionp->getRegionID().notNull())
	addRegion(regionp->getName(), U64_to_str(regionp->getHandle()), regionp->getRegionID());

	if (!we_want_notifications) return;
	LLHost host = regionp->getHost();
	LLChat chat;
	chat.mText = llformat("Removed region '%s' - Host:%s Colo:%s ClassID:%i cpuRatio:%i SKU:%s Type:%s",
		regionp->getName().c_str(),
		regionp->getHost().getIPandPort().c_str(),
		regionp->getSimColoName().c_str(),
		regionp->getSimClassID(),
		regionp->getSimCPURatio(),
		regionp->getSimProductSKU().c_str(),
		regionp->getLocalizedSimProductName().c_str());
	chat.mSourceType = CHAT_SOURCE_SYSTEM;
	LLFloaterChat::addChat(chat);
}

// --------------------------------------------------------------------------
OSCircuits::OSCircuits(const LLSD&) 
: LLFloater(std::string("Region Circuits")),
	mSelectedRegion(0),
	mSelectedRegionhandle(0),
	mSelectedRegionId(LLUUID::null),
	mRegionId(LLUUID::null)
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "os_floater_region_circuits.xml", NULL, false);
}

OSCircuits::~OSCircuits()
{
}

BOOL OSCircuits::postBuild()
{
	mRegionScrollList=getChild<LLScrollListCtrl>("region_list");
	mRegionScrollList->setCommitCallback(boost::bind(&OSCircuits::onSelectRegion,this));

	mDisconnectButton=getChild<LLButton>("disconnect_btn");
	mDisconnectAllNeighborsButton=getChild<LLButton>("disconnect_all_btn");
	
	mDisconnectButton->setCommitCallback(boost::bind(&OSCircuits::onDisconnectPressed,this));
	mDisconnectAllNeighborsButton->setCommitCallback(boost::bind(&OSCircuits::onDisconnectAllNeighbors,this));

	update();// request list of recent regions

	return TRUE;
}

void OSCircuits::onSelectRegion()
{
	LLScrollListItem* item=mRegionScrollList->getFirstSelected();
	if(!item)
	{
		return;
	}

	S32 column=mRegionScrollList->getColumn("region_id")->mIndex;
	mSelectedRegionId=item->getColumn(column)->getValue().asUUID();

	column=mRegionScrollList->getColumn("region_handle")->mIndex;
	mSelectedRegionhandle=str_to_U64(item->getColumn(column)->getValue().asString());

	column=mRegionScrollList->getColumn("region_name")->mIndex;
	mSelectedName=item->getColumn(column)->getValue().asString();

	LLViewerRegion *regionp = LLWorld::getInstance()->getRegionFromID(mSelectedRegionId);
	if(regionp)
	{
		mSelectedRegion=regionp;
	}
}

void OSCircuits::onDisconnectPressed()
{
	if(mSelectedRegion)
	{
		LLHost host = mSelectedRegion->getHost();
		LLCircuitData* cdp = gMessageSystem->mCircuitInfo.findCircuit(mSelectedRegion->getHost());
		if(cdp)
		{
			gMessageSystem->newMessageFast(_PREHASH_CloseCircuit);
			gMessageSystem->sendReliable(cdp->getHost());
		}
		
		LLChat chat;
		chat.mText=llformat("Removed region '%s' - Host:%s", mSelectedName.c_str(),host.getIPandPort().c_str());
		chat.mSourceType = CHAT_SOURCE_SYSTEM;
		LLFloaterChat::addChat(chat);
		
		LLWorld::getInstance()->removeRegion(host);

		//update();
	}
}

void OSCircuits::onDisconnectAllNeighbors()
{
	for (LLWorld::region_list_t::const_iterator iter = LLWorld::getInstance()->getRegionList().begin();
		 iter != LLWorld::getInstance()->getRegionList().end(); ++iter)
	{
		LLViewerRegion* regionp = *iter;
		LLHost host = regionp->getHost();
		std::string name = regionp->getName();

		if (regionp != gAgent.getRegion())
		{
			LLCircuitData* cdp = gMessageSystem->mCircuitInfo.findCircuit(host);
			if(cdp)
			{
				gMessageSystem->newMessageFast(_PREHASH_CloseCircuit);
				gMessageSystem->sendReliable(cdp->getHost());
			}

			LLChat chat;
			chat.mText=llformat("Removed region '%s' - Host:%s", mSelectedName.c_str(),host.getIPandPort().c_str());
			chat.mSourceType = CHAT_SOURCE_SYSTEM;
			LLFloaterChat::addChat(chat);

			LLWorld::getInstance()->removeRegion(host);
		}
	}
	//update();
}

void OSCircuits::draw()
{
	LLFloater::draw();
}

void OSCircuits::update()
{
	mRegionScrollList->deleteAllItems();
	RecentRegionList::instance().requestList(this);
}

void OSCircuits::addRegionList(const std::string& name, const std::string& handle, const LLUUID& regionid)
{
	// insert the region into the scroll list

	LLSD item;
	item["columns"][0]["column"]="region_name";
	item["columns"][0]["value"]= name;
	item["columns"][1]["column"]="region_handle";
	item["columns"][1]["value"]= handle;
	item["columns"][2]["column"]="region_id";
	item["columns"][2]["value"]= regionid;

	mRegionScrollList->addElement(item,ADD_TOP);
}

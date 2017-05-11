/** 
 * @file os_floaterexploreparticles.cpp
 * @brief Explores Objects Emmiting Particles and gives you the ability to reverse engineer the scripts or just inspect spam emmiting objects.
 *
 **/

#include "llviewerprecompiledheaders.h"

#include "llagent.h"
#include "llagentcamera.h"
#include "llapr.h"
#include "llbbox.h"
#include "llbutton.h"
#include "llcategory.h"
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "lldatapacker.h"
#include "lldir.h"
#include "lldrawable.h"
#include "lldrawpoolavatar.h"
#include "llfloaterchat.h"
#include "llfloateravatarinfo.h"
#include "llfloaterperms.h"
#include "llfloatertools.h"
#include "llfocusmgr.h"
#include "llinventorymodel.h"
#include "llpreviewsound.h"
#include "llscrolllistctrl.h"
#include "llselectmgr.h"
#include "llsliderctrl.h"
#include "llspinctrl.h"
#include "llstring.h"
#include "lltextbox.h"
#include "lltoolcomp.h"
#include "lltoolmgr.h"
#include "lltracker.h"
#include "lluuid.h"
#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "llviewermessage.h"
#include "llviewermenufile.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"
#include "llviewerpartsource.h"
#include "llviewerwindow.h"
#include "llviewerregion.h"
#include "llviewertexture.h"
#include "llviewerwindow.h"
#include "os_floaterinspecttexture.h"
#include "llnotificationsutil.h"

#include "os_floaterexploreparticles.h"

LLScrollListCtrl* OSParticleExplorer::mTheList;
LLTextureCtrl* OSParticleExplorer::texture_ctrl;

std::map<LLUUID, ObjectDetails> OSParticleExplorer::mRequests;
std::map<LLUUID, OSParticleListEntry> OSParticleExplorer::mParticleObjectList;

OSParticleListEntry::OSParticleListEntry(const std::string& OwnerName, const std::string& ObjectName, const LLUUID& TextureID) :
		mOwnerName(OwnerName), mObjectName(ObjectName), mTextureID(TextureID)
{
} 

void OSParticleListEntry::setAvatarName(std::string OwnerName)
{
	if (OwnerName.empty())
	{
		return;
	}
	mOwnerName = OwnerName;
}

std::string OSParticleListEntry::getAvatarName()
{
	return mOwnerName;
}

std::string OSParticleListEntry::getObjectName()
{
	return mObjectName;
}

void OSParticleListEntry::setObjectName(std::string ObjectName)
{
	if (ObjectName.empty())
	{
		LL_WARNS()<< "Trying to set null id1" << LL_ENDL;
		return;
	}
	mObjectName = ObjectName;
}

LLUUID OSParticleListEntry::getTextureID()
{
	return mTextureID;
}

void OSParticleListEntry::setTextureID(LLUUID TextureID)
{
	if (TextureID.isNull())
	{ 
		LL_WARNS()<< "Trying to set null id1" << LL_ENDL;
		return;
	}
	mTextureID = TextureID;
}


//####################################################
//    Create Instance
//####################################################

OSParticleExplorer* OSParticleExplorer::sInstance = NULL;

OSParticleExplorer::OSParticleExplorer() :  LLFloater()
{
	llassert_always(sInstance == NULL);
	sInstance = this;
}

OSParticleExplorer::~OSParticleExplorer()
{
	sInstance = NULL;
}

void OSParticleExplorer::toggle()
{
	if (sInstance)
	{
		if(sInstance->getVisible())
		{
			sInstance->setVisible(FALSE);
		}
		else
		{
			sInstance->setVisible(TRUE);
		}
	}
	else
	{
		sInstance = new OSParticleExplorer();
		LLUICtrlFactory::getInstance()->buildFloater(sInstance, "os_floaterexploreparticles.xml");
	}
}

void OSParticleExplorer::draw()
{
	//results();
	LLFloater::draw();
}


void OSParticleExplorer::close(bool app)
{
	if(app)
	{
		LLFloater::close(app);
	}else
	{
		if(sInstance)
		{
			sInstance->setVisible(FALSE);
		}
	}
}

void OSParticleExplorer::refresh()
{
	mTheList->deleteAllItems();
	mParticleObjectList.clear();
	mRequests.clear();

	LLSelectMgr::getInstance()->deselectAll();
	//open Tool Manager (Edit) to select all
	if (!LLToolMgr::getInstance()->inBuildMode())
	{
		gFloaterTools->open();
		LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
		gFloaterTools->setEditTool( LLToolCompTranslate::getInstance() );
	}

	S32 i; S32 total = gObjectList.getNumObjects();
	LLViewerRegion* our_region = gAgent.getRegion();
	LLSelectMgr::getInstance()->deselectAll();

	//BACKUP SelectOwned Value
	BOOL SelectionSettingBackup = gSavedSettings.getBOOL("SelectOwnedOnly");
	//Set to false to select all objects
	gSavedSettings.setBOOL("SelectOwnedOnly", FALSE);

	for (i = 0; i < total; i++)
	{
		LLViewerObject *objectp = gObjectList.getObject(i);
		if (objectp)
		{
			if ((objectp->getRegion() == our_region) && objectp->isParticleSource())
			{
				LLSelectMgr::getInstance()->selectObjectAndFamily(objectp);
				
				ObjectDetails* details = &sInstance->mRequests[objectp->getID()];
							details->id = objectp->getID();
							details->Texture = objectp->mPartSourcep->getImageUUID();
				
				LLSelectMgr::getInstance()->deselectHighlightedObjects();
				LLSelectMgr::getInstance()->deselectAll();
			}
		}
	}
	gFloaterTools->close(true);

	//SET BACK TO Settings as it was before
	gSavedSettings.setBOOL("SelectOwnedOnly", SelectionSettingBackup);
}

BOOL OSParticleExplorer::postBuild()
{
	mTheList = getChild<LLScrollListCtrl>("object_list");
	mTheList->setDoubleClickCallback(onDoubleClick,this);
	texture_ctrl = getChild<LLTextureCtrl>("imagette");
	childSetAction("reload", onClickReload, this);
	childSetAction("copy", onClickCopy, this);
	childSetAction("beacon", onClickBeacon, this);
	childSetAction("return", onClickReturn, this);
	childSetCommitCallback("object_list", onSelectObject);

	refresh();

	return true;
}

void OSParticleExplorer::onClickReload(void* user_data)
{
	sInstance->refresh();
}

// static
void OSParticleExplorer::onDoubleClick(void *userdata)
{
	LLScrollListItem *item = mTheList->getFirstSelected();
	if (!item) return;
	LLUUID object_id = item->getUUID();
	LLViewerObject* objectp = gObjectList.findObject(object_id);
	if (objectp)
	{
		//LLTracker::trackLocation(objectp->getPositionGlobal(), mParticleObjectList[object_id].getObjectName(), "", LLTracker::LOCATION_ITEM);
		// zoom in on object center instead of where we clicked, as we need to see the manipulator handles
		if (gAgentCamera.lookAtObject(object_id, false))
		{
			return;
		}
		
	}
}

void OSParticleExplorer::onClickBeacon(void* user_data)
{
	LLScrollListItem *item = mTheList->getFirstSelected();
	if (!item) return;
	LLUUID object_id = item->getUUID();
	LLViewerObject* objectp = gObjectList.findObject(object_id);
	if (objectp)
	{
		LLTracker::trackLocation(objectp->getPositionGlobal(), mParticleObjectList[object_id].getObjectName(), "", LLTracker::LOCATION_ITEM);
	}
}

void OSParticleExplorer::onClickReturn(void* user_data)
{

	LLScrollListItem *item = mTheList->getFirstSelected();
	if (!item) return;
	LLUUID object_id = item->getUUID();
	LLViewerObject* objectp = gObjectList.findObject(object_id);
	if (objectp)
	{
		//BACKUP SelectOwned Value
		BOOL SelectionSettingBackup = gSavedSettings.getBOOL("SelectOwnedOnly");
		//Set to false to select all objects
		gSavedSettings.setBOOL("SelectOwnedOnly", FALSE);

		LLSelectMgr::getInstance()->deselectAll();
		//open Tool Manager (Edit) to select all
		if (!LLToolMgr::getInstance()->inBuildMode())
		{
			gFloaterTools->open();
			LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
			gFloaterTools->setEditTool( LLToolCompTranslate::getInstance() );
		}

		LLSelectMgr::getInstance()->selectObjectAndFamily(objectp);


		//SET BACK TO Settings as it was before
		gSavedSettings.setBOOL("SelectOwnedOnly", SelectionSettingBackup);
	}

}

void OSParticleExplorer::onClickCopy(void* user_data)
{
	LLScrollListItem *item = mTheList->getFirstSelected();
	if (!item) return;
	LLUUID object_id = item->getUUID();
	LLViewerObject* objectp = gObjectList.findObject(object_id);

	if(!objectp) return;

	LLNotificationsUtil::add("ScriptRipFunction",
								LLSD(),
								LLSD(),
								boost::bind(&LLFloaterInspectTexture::particle_rip, _1, _2, objectp));
}

void OSParticleExplorer::onSelectObject(LLUICtrl* ctrl, void* user_data)
{
	LLUUID TextureID;

	if(mTheList->getAllSelected().size() > 0)
	{
		LLScrollListItem* first_selected = mTheList->getFirstSelected();
		if (first_selected)
		{
			TextureID = mParticleObjectList[first_selected->getUUID()].getTextureID();
		}
	}
	else
	{
		TextureID=LLUUID::null;
	}

	sInstance->texture_ctrl->setImageAssetID(TextureID);

}

void OSParticleExplorer::processObjectProperties(LLMessageSystem* msg, void** user_data)
{
	S32 i;
	S32 count = msg->getNumberOfBlocksFast(_PREHASH_ObjectData);
	for (i = 0; i < count; i++)
	{
		LLUUID id;
		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_ObjectID, id, i);

		LLUUID creator_id;
		LLUUID owner_id;
		LLUUID group_id;
		LLUUID last_owner_id;
		U64 creation_date;
		LLUUID extra_id;
		U32 base_mask, owner_mask, group_mask, everyone_mask, next_owner_mask;
		LLSaleInfo sale_info;
		LLCategory category;
		LLAggregatePermissions ag_perms;
		LLAggregatePermissions ag_texture_perms;
		LLAggregatePermissions ag_texture_perms_owner;
		
		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_CreatorID, creator_id, i);
		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_OwnerID, owner_id, i);
		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_GroupID, group_id, i);
		msg->getU64Fast(_PREHASH_ObjectData, _PREHASH_CreationDate, creation_date, i);
		msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_BaseMask, base_mask, i);
		msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_OwnerMask, owner_mask, i);
		msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_GroupMask, group_mask, i);
		msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_EveryoneMask, everyone_mask, i);
		msg->getU32Fast(_PREHASH_ObjectData, _PREHASH_NextOwnerMask, next_owner_mask, i);
		sale_info.unpackMultiMessage(msg, _PREHASH_ObjectData, i);

		ag_perms.unpackMessage(msg, _PREHASH_ObjectData, _PREHASH_AggregatePerms, i);
		ag_texture_perms.unpackMessage(msg, _PREHASH_ObjectData, _PREHASH_AggregatePermTextures, i);
		ag_texture_perms_owner.unpackMessage(msg, _PREHASH_ObjectData, _PREHASH_AggregatePermTexturesOwner, i);
		category.unpackMultiMessage(msg, _PREHASH_ObjectData, i);

		S16 inv_serial = 0;
		msg->getS16Fast(_PREHASH_ObjectData, _PREHASH_InventorySerial, inv_serial, i);

		LLUUID item_id;
		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_ItemID, item_id, i);
		LLUUID folder_id;
		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_FolderID, folder_id, i);
		LLUUID from_task_id;
		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_FromTaskID, from_task_id, i);

		msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_LastOwnerID, last_owner_id, i);

		std::string ObjectName;
		msg->getStringFast(_PREHASH_ObjectData, _PREHASH_Name, ObjectName, i);
		std::string ObjectDesc;
		msg->getStringFast(_PREHASH_ObjectData, _PREHASH_Description, ObjectDesc, i);

		if(mRequests.count( id ) != 0)
		{
			ObjectDetails* details = &mRequests[id];
			LLUUID TextureID = details->Texture;

			std::vector <LLUUID> selected = sInstance->mTheList->getSelectedIDs();
			S32 scrollpos = sInstance->mTheList->getScrollPos();
			std::string OwnerName = "searching...";
			gCacheName->getFullName(owner_id, OwnerName);

			LLSD element;
			element["id"] = id;
			element["columns"][LIST_OBJECT_OWNER]["column"] = "particle_owner";
			element["columns"][LIST_OBJECT_OWNER]["type"] = "text";
			element["columns"][LIST_OBJECT_OWNER]["color"] = gColors.getColor("DefaultListText").getValue();
			element["columns"][LIST_OBJECT_OWNER]["value"] = OwnerName.c_str();

			element["columns"][LIST_PARTICLE_SOURCE]["column"] = "particle_source";
			element["columns"][LIST_PARTICLE_SOURCE]["type"] = "text";
			element["columns"][LIST_PARTICLE_SOURCE]["color"] = gColors.getColor("DefaultListText").getValue();
			element["columns"][LIST_PARTICLE_SOURCE]["value"] = ObjectName.c_str();

			element["columns"][LIST_SOURCE_TEXTURE]["column"] = "particle_texture";
			element["columns"][LIST_SOURCE_TEXTURE]["type"] = "text";
			element["columns"][LIST_SOURCE_TEXTURE]["color"] = gColors.getColor("DefaultListText").getValue();
			element["columns"][LIST_SOURCE_TEXTURE]["value"] = TextureID.asString().c_str();

			mTheList->addElement(element, ADD_BOTTOM);
			//Add all Values to the List
			OSParticleListEntry entry(OwnerName, ObjectName, TextureID);
			mParticleObjectList[id] = entry;
			
			mTheList->sortByColumn("Owner", TRUE);
			mTheList->setScrollPos(scrollpos);
			mRequests.erase(id);
		}
	}
}

/*
 * 
 * @file os_floaterobjectfunctions.h
 * @author simms
 *
 * ----------------------------------------------------------------------------
 * "The DILLIGAFF license" (Do I Look Like I Give A Flying Fuck):
 * Copyright (c) 2012, Simms, Inc.
 * 
 * If you can throw me some shrapnel for beer and coffee https://flattr.com/donation/give/to/simms
 * ----------------------------------------------------------------------------
 */

#include "llviewerprecompiledheaders.h"
#include "os_floaterobjectfunctions.h"
#include "lluictrlfactory.h"

#include "llbutton.h"
#include "llcombobox.h"
#include "stdenums.h"

#include "llagent.h"
#include "llvoavatarself.h"
#include "llselectmgr.h"
#include "llviewermenu.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"
#include "llinventoryfunctions.h"
#include "llviewerregion.h"
#include "llhost.h"

struct OSDuplicateData
{
	LLVector3	offset;
	U32			flags;
};

LLFloaterObjectFunctions::LLFloaterObjectFunctions(const LLSD& seed)
{
	mDirty = true;
	LLUICtrlFactory::getInstance()->buildFloater(this, "os_floater_objectfunctions.xml");
}

LLFloaterObjectFunctions::~LLFloaterObjectFunctions(void)
{
}

BOOL LLFloaterObjectFunctions::postBuild()
{
	mBtnTakeCopy = getChild<LLButton>("takecopy_obj");
	mBtnTake = getChild<LLButton>("take_obj");
	mBtnDuplicateObj = getChild<LLButton>("duplicate_obj");
	mBtnDelete = getChild<LLButton>("delete_obj");
	mBtnReturn = getChild<LLButton>("return_obj");
	mBtnBlink = getChild<LLButton>("blink_btn");
	mBtnLinkObj = getChild<LLButton>("link_obj");
	mBtnUnlinkObj = getChild<LLButton>("unlink_obj");
	mBtnTextures = getChild<LLButton>("texture_obj");
	mBtnExportXml = getChild<LLButton>("export_xml");
	mBtnTouch = getChild<LLButton>("touch_obj");
	mBtnSit = getChild<LLButton>("sit_obj");

	mBtnTakeCopy->setClickedCallback(boost::bind(&LLFloaterObjectFunctions::onClickTakeCopy, this));
	mBtnTake->setClickedCallback(boost::bind(&LLFloaterObjectFunctions::onClickTake, this));
	mBtnDuplicateObj->setClickedCallback(boost::bind(&LLFloaterObjectFunctions::onClickDuplicate, this));
	mBtnDelete->setClickedCallback(boost::bind(&LLFloaterObjectFunctions::onClickDelete, this));
	mBtnReturn->setClickedCallback(boost::bind(&LLFloaterObjectFunctions::onClickReturn, this));
	mBtnBlink->setClickedCallback(boost::bind(&LLFloaterObjectFunctions::onClickBlink, this));
	mBtnLinkObj->setClickedCallback(boost::bind(&LLFloaterObjectFunctions::onClickLinkObj, this));
	mBtnUnlinkObj->setClickedCallback(boost::bind(&LLFloaterObjectFunctions::onClickUnlinkObj, this));
	mBtnTextures->setClickedCallback(boost::bind(&LLFloaterObjectFunctions::onClickTextures, this));
	mBtnExportXml->setClickedCallback(boost::bind(&LLFloaterObjectFunctions::onClickExportXml, this));
	mBtnTouch->setClickedCallback(boost::bind(&LLFloaterObjectFunctions::onClickTouch, this));
	mBtnSit->setClickedCallback(boost::bind(&LLFloaterObjectFunctions::onClickSit, this));

	LLSelectMgr::getInstance()->mUpdateSignal.connect(boost::bind(&LLFloaterObjectFunctions::updateSelection, this));

	return TRUE;
}

void LLFloaterObjectFunctions::onOpen()
{
	LLObjectSelectionHandle object_selection = LLSelectMgr::getInstance()->getSelection();
	if (!(object_selection->getPrimaryObject()))
	{
		close();
		return;
	}
	mObjectSelection = LLSelectMgr::getInstance()->getEditSelection();
	dirty();
}

void LLFloaterObjectFunctions::updateSelection()
{
	LLObjectSelectionHandle object_selection = LLSelectMgr::getInstance()->getSelection();
	LLSelectNode* node = object_selection->getFirstRootNode();

	if (node && !node->mValid)
	{
		return;
	}

	mObjectSelection = object_selection;
	dirty();
}

void LLFloaterObjectFunctions::dirty()
{
	mDirty = true;
}

void LLFloaterObjectFunctions::draw()
{
	if (mDirty)
	{
		refresh();
		mDirty = false;
	}
	LLFloater::draw();
}

void LLFloaterObjectFunctions::refresh()
{
	if (mObjectSelection->isEmpty()) 
	{
		mBtnTakeCopy->setEnabled(false);
		mBtnTake->setEnabled(false);
		mBtnDuplicateObj->setEnabled(false);
		mBtnDelete->setEnabled(false);
		mBtnReturn->setEnabled(false);
		mBtnBlink->setEnabled(false);
		mBtnLinkObj->setEnabled(false);
		mBtnUnlinkObj->setEnabled(false);
		mBtnTextures->setEnabled(false);
		mBtnExportXml->setEnabled(false);
		mBtnTouch->setEnabled(false);
		mBtnSit->setEnabled(false);

	}
	else
	{
		mBtnTakeCopy->setEnabled(enable_object_take_copy());
		mBtnTake->setEnabled(visible_take_object());
		mBtnDuplicateObj->setEnabled(LLSelectMgr::getInstance()->canDuplicate());
		mBtnDelete->setEnabled(enable_object_delete());
		mBtnReturn->setEnabled(enable_object_return());
		mBtnBlink->setEnabled(enable_object_delete());

		for (LLObjectSelection::valid_iterator iter = mObjectSelection->valid_begin(); iter != mObjectSelection->valid_end(); iter++)
		{
			LLSelectNode* obj = *iter;
			if (obj->mCreationDate == 0)
			{
				continue;
			}

			if (obj->getObject()->isRoot())
			{
				mBtnTextures->setEnabled(true);
				mBtnExportXml->setEnabled(true);
				mBtnTouch->setEnabled(true);
				mBtnSit->setEnabled(true);

				LLViewerObject* objectp = obj->getObject();
				mBtnLinkObj->setEnabled(LLSelectMgr::getInstance()->enableLinkObjects());
				LLViewerObject* linkset_parent = objectp->getSubParent() ? objectp->getSubParent() : objectp;
				mBtnUnlinkObj->setEnabled(
					LLSelectMgr::getInstance()->enableUnlinkObjects()
					&& (linkset_parent->numChildren() >= 1)
					&& LLSelectMgr::getInstance()->getSelection()->getRootObjectCount() <= 1);
			}
		}
	}
}

void LLFloaterObjectFunctions::onClickTakeCopy()
{
	handle_take_copy();
}

void LLFloaterObjectFunctions::onClickTake()
{
	handle_take();
}

void LLFloaterObjectFunctions::onClickDuplicate()
{
	LLViewerObject* selected_objectp = mObjectSelection->getFirstRootObject();
	if (selected_objectp)
	{
		LLVector3 scale = selected_objectp->getScale();
		LLVector3 offset = LLVector3(0.0f, 0.0f, scale.mV[VZ]);
		OSDuplicateData	data;
		data.offset = offset;
		data.flags = FLAGS_CREATE_SELECTED;
		LLSelectMgr::getInstance()->sendListToRegions("ObjectDuplicate", LLSelectMgr::getInstance()->packDuplicateHeader, LLSelectMgr::getInstance()->packDuplicate, &data, SEND_ONLY_ROOTS);
	}
	// the new copy will be coming in selected
	LLSelectMgr::getInstance()->deselectAll();
}
void LLFloaterObjectFunctions::onClickDelete()
{
	handle_object_delete();
}
void LLFloaterObjectFunctions::onClickReturn()
{
	handle_object_return();
}
void LLFloaterObjectFunctions::onClickBlink()
{
	// move current selection based on delta from position and update z position
	for (LLObjectSelection::root_iterator iter = LLSelectMgr::getInstance()->getSelection()->root_begin();
		iter != LLSelectMgr::getInstance()->getSelection()->root_end(); iter++)
	{
		LLSelectNode* node = *iter;
		if (node)
		{
			LLVector3d cur_pos = node->getObject()->getPositionGlobal();
			LLVector3d new_pos = cur_pos.mdV[VZ] = 340282346638528859811704183484516925440.0f;
			node->mDuplicatePos = node->getObject()->getPositionGlobal();
			node->getObject()->setPositionGlobal(new_pos);
		}
	}

	LLSelectMgr::getInstance()->sendMultipleUpdate(UPD_POSITION);
}

void LLFloaterObjectFunctions::onClickLinkObj()
{
	LL_INFOS() << "Attempting link." << LL_ENDL;
	LLSelectMgr::getInstance()->linkObjects();
}

void LLFloaterObjectFunctions::onClickUnlinkObj()
{
	LL_INFOS() << "Attempting unlink." << LL_ENDL;
	LLSelectMgr::getInstance()->unlinkObjects();
}

void LLFloaterObjectFunctions::onClickTextures()
{
	show_floater("inspect textures");
}

void LLFloaterObjectFunctions::onClickExportXml()
{
	show_floater("export list");
}

void LLFloaterObjectFunctions::onClickTouch()
{
	handle_object_touch();
}

void LLFloaterObjectFunctions::onClickSit()
{
	LLViewerObject* object = mObjectSelection->getFirstRootObject();
	if (object)
	{
		if (object->isRoot())
		{

			if (gAgentAvatarp->isSitting() && gAgentAvatarp->getRoot() == object)
			{
				gAgent.standUp();
				return;
			}

			LLVector3 scale = object->getScale();
			LLVector3 offset = LLVector3(0.0f, 0.0f, scale.mV[VZ]);
			gReSitTargetID = object->mID;
			gReSitOffset = offset;
			gMessageSystem->newMessageFast(_PREHASH_AgentRequestSit);
			gMessageSystem->nextBlockFast(_PREHASH_AgentData);
			gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			gMessageSystem->nextBlockFast(_PREHASH_TargetObject);
			gMessageSystem->addUUIDFast(_PREHASH_TargetID, object->mID);
			gMessageSystem->addVector3Fast(_PREHASH_Offset, LLVector3::zero);

			object->getRegion()->sendReliableMessage();
		}
	}
}
/** 
 * @file llfloaterinspecttexture.cpp
 * @brief Texure Inspection Floater
 *
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Skills Hak), Simms(h20) And Cinder Roxley wrote this file. As long as you retain this notice you can do
 * whatever you want with this stuff. If we meet some day, and you think this
 * stuff is worth it, you can buy Us a beer in return http://skills.tumblr.com/ , https://flattr.com/donation/give/to/simms , <cinder@cinderblocks.biz>
 * ----------------------------------------------------------------------------
 */

#include "llviewerprecompiledheaders.h"

#include "os_floaterinspecttexture.h"
#include "llagent.h"
#include "llfloatertools.h"
#include "llfocusmgr.h"
#include "llscrolllistctrl.h"
#include "llscrolllistitem.h"
#include "llselectmgr.h"
#include "lltoolcomp.h"
#include "lltoolmgr.h"
#include "llviewercontrol.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"
#include "llviewertexture.h"
#include "llpartdata.h"
#include "llviewerpartsource.h"
#include "lltexturectrl.h"
#include "llnotificationsutil.h"
#include "llinventorypanel.h"
#include "llinventorydefines.h"
#include "llviewerinventory.h"

#include "llviewertexturelist.h"
#include "os_invtools.h"
#include "llviewertextureanim.h"
#include "llvovolume.h"
#include "llviewermenufile.h"
#include "llfloaterchat.h"
#include "llclipboard.h"
#include "llnotifications.h"//choice

LLFloaterInspectTexture::LLFloaterInspectTexture(const LLSD&) 
: LLFloater(std::string("Inspect Textures")),
	mObjectID(LLUUID::null),
	mIsAvatar(false),
	mDirty(true)
{
	mCommitCallbackRegistrar.add("Inspect.CpToInvSelected",	boost::bind(&LLFloaterInspectTexture::onClickCpToInvSelected, this));
	mCommitCallbackRegistrar.add("Inspect.CpToInvAll",	boost::bind(&LLFloaterInspectTexture::onClickCpToInvAll, this));
	mCommitCallbackRegistrar.add("Inspect.SelectObject",	boost::bind(&LLFloaterInspectTexture::onSelectTexture, this));
	mCommitCallbackRegistrar.add("Inspect.RipAnimScript",	boost::bind(&LLFloaterInspectTexture::onClickRipTextureAnim, this));
	mCommitCallbackRegistrar.add("Inspect.RipParticleScript",	boost::bind(&LLFloaterInspectTexture::onClickRipParticle, this));
	mCommitCallbackRegistrar.add("Inspect.CpUuid", boost::bind(&LLFloaterInspectTexture::onClickCpUuid, this));
	mCommitCallbackRegistrar.add("Inspect.CpUuidAll", boost::bind(&LLFloaterInspectTexture::onClickCpAllUuid, this));
	LLUICtrlFactory::getInstance()->buildFloater(this, "os_floater_inspect_texture.xml", NULL, false);
}

LLFloaterInspectTexture::~LLFloaterInspectTexture(void)
{
}

BOOL LLFloaterInspectTexture::postBuild()
{
	mObjectID = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject()->getID();
	mTextureList = getChild<LLScrollListCtrl>("object_list");
	mTextureList->sortByColumn(std::string("types"), TRUE);
	texture_ctrl = getChild<LLTextureCtrl>("imagette");
	if (mObjectID.isNull())
	{
		mIsAvatar = false;
		LLSelectMgr::getInstance()->mUpdateSignal.connect(boost::bind(&LLFloaterInspectTexture::updateSelection, this));
	}
	else
	{
		LLViewerObject* obj = gObjectList.findObject(mObjectID);

		if (obj && obj->isAvatar())
		{
			mIsAvatar = true;
		}
		else
		{
			mIsAvatar = false;
			LLSelectMgr::getInstance()->mUpdateSignal.connect(boost::bind(&LLFloaterInspectTexture::updateSelection, this));
		}
	}

	mStatsMemoryTotal = 0;
	return TRUE;
}

void LLFloaterInspectTexture::onOpen()
{
	LLObjectSelectionHandle object_selection = LLSelectMgr::getInstance()->getSelection();
	if (!mIsAvatar)
	{
		if(!(object_selection->getPrimaryObject()))
		{
			close();
			return;
		}
		mObjectSelection = LLSelectMgr::getInstance()->getEditSelection();
	}
	dirty();
}

void LLFloaterInspectTexture::updateSelection()
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

void LLFloaterInspectTexture::dirty()
{
	mDirty = true;
}

void LLFloaterInspectTexture::draw()
{
	if (mDirty)
	{
		refresh();
		mDirty = false;
	}
	LLFloater::draw();
}

void LLFloaterInspectTexture::onSelectTexture()
{
	dirty();
}

LLUUID LLFloaterInspectTexture::getSelectedUUID()
{
	if (mTextureList->getAllSelected().size() > 0)
	{
		LLScrollListItem* first_selected = mTextureList->getFirstSelected();
		if (first_selected)
		{
			return first_selected->getUUID();
		}
		
	}
	return LLUUID::null;
}

LLUUID findUploader(LLViewerTexture* img)
{
	LLViewerFetchedTexture* imagep = dynamic_cast<LLViewerFetchedTexture*>(img);
	//LLPointer<LLViewerFetchedTexture> imagep(static_cast<LLViewerFetchedTexture*>(img));
	if (imagep && imagep->mComment.find("a") != imagep->mComment.end())
	{
		return LLUUID(imagep->mComment["a"]);
	}
	return LLUUID::null;
}

void LLFloaterInspectTexture::refresh()
{
	unique_textures.clear();

	mStatsMemoryTotal = 0;
	mStatsText = "Texture stats\n\n";
	childSetValue("stats_text", mStatsText);

	S32 objcount = 0;
	S32 primcount = 0;
	S32 facecount = 0;
	S32 pos = mTextureList->getScrollPos();
	getChildView("button selection")->setEnabled(false);
	getChildView("ripanim")->setEnabled(false);

	LLUUID selected_uuid = getSelectedUUID();
	S32 selected_index = mTextureList->getFirstSelectedIndex();

	if(selected_index < 0)
	{
		mTextureList->selectNthItem(1);
		selected_index = mTextureList->getFirstSelectedIndex();
	}

	mTextureList->operateOnAll(LLScrollListCtrl::OP_DELETE);

	if (mIsAvatar)
	{
		// attachments
		LLViewerObject* obj = gObjectList.findObject(mObjectID);
		if (obj)
		{
			LLVOAvatar* av = (LLVOAvatar*)obj;
			if (!av) return;
			LLViewerObject::child_list_t child_list = av->getChildren();

			S32 height;
			S32 width;
			S32 components;
			std::string type_str;
			LLUUID uuid;
			LLUUID uploader;
			LLViewerTexture* img;

			img = av->getTEImage(20);
			if (img)
			{
				uuid = img->getID();
				height = img->getHeight();
				width = img->getWidth();
				components = img->getComponents();
				type_str = "*Baked Hair";
				uploader = findUploader(img);
				addToList(uuid, uploader, NULL, height, width, components, type_str);
			}

			img = av->getTEImage(8);
			if (img)
			{
				uuid = img->getID();
				height = img->getHeight();
				width = img->getWidth();
				components = img->getComponents();
				type_str = "*Baked Head";
				uploader = findUploader(img);
				addToList(uuid, uploader, NULL, height, width, components, type_str);
			}

			img = av->getTEImage(11);
			if (img)
			{
				uuid = img->getID();
				height = img->getHeight();
				width = img->getWidth();
				components = img->getComponents();
				type_str = "*Baked Eyes";
				uploader = findUploader(img);
				addToList(uuid, uploader, NULL, height, width, components, type_str);
			}

			img = av->getTEImage(9);
			if (img)
			{
				uuid = img->getID();
				height = img->getHeight();
				width = img->getWidth();
				components = img->getComponents();
				type_str = "*Baked Upper";
				uploader = findUploader(img);
				addToList(uuid, uploader, NULL, height, width, components, type_str);
			}

			img = av->getTEImage(10);
			if (img)
			{
				uuid = img->getID();
				height = img->getHeight();
				width = img->getWidth();
				components = img->getComponents();
				type_str = "*Baked Lower";
				uploader = findUploader(img);
				addToList(uuid, uploader, NULL, height, width, components, type_str);
			}

			img = av->getTEImage(19);
			if (img)
			{
				uuid = img->getID();
				height = img->getHeight();
				width = img->getWidth();
				components = img->getComponents();
				type_str = "*Baked Skirt";
				uploader = findUploader(img);
				addToList(uuid, uploader, NULL, height, width, components, type_str);
			}

			for (LLViewerObject::child_list_t::iterator i = child_list.begin(); i != child_list.end(); ++i)
			{
				LLViewerObject* rootp = *i;

				if (rootp->isRoot())
				{
					if (rootp->isHUDAttachment()) continue; //huds don't really matter m8
					objcount++;
				}
				else
				{
					if(rootp->getSubParent() && rootp->getSubParent()->isAvatar())
					{
						objcount++;
					}
				}

				primcount++;
				U8 te_count = rootp->getNumTEs();
				facecount += te_count;

				iterateObjects(rootp,te_count);

				LLViewerObject::child_list_t select_list = rootp->getChildren();
				LLViewerObject::child_list_t::iterator select_iter;
				for (select_iter = select_list.begin(); select_iter != select_list.end(); ++select_iter)
				{
					LLViewerObject* childp = (*select_iter);
					if (childp)
					{
						primcount++;
						te_count = childp->getNumTEs();
						facecount += te_count;
						iterateObjects(childp,te_count);
					}
				}
			}
		}
	}
	else
	{
		for (LLObjectSelection::valid_iterator iter = mObjectSelection->valid_begin(); iter != mObjectSelection->valid_end(); iter++)
		{
			LLSelectNode* obj = *iter;
			if (obj->mCreationDate == 0)
			{	
				continue;
			}

			if (obj->getObject()->isRoot())
			{	
				objcount++;
			}
			else
			{
				if(obj->getObject()->getSubParent() && obj->getObject()->getSubParent()->isAvatar())
				{
					objcount++;
				}
			}

			primcount++;
			U8 te_count = obj->getObject()->getNumTEs();
			facecount += te_count;

			iterateObjects(obj->getObject(),te_count);
		}
	}
	
	if(mTextureList->getItemIndex(selected_uuid) != selected_index)
	{
		selected_index = 1;
	}
	mTextureList->selectNthItem(selected_index);

	bool hasanim = false;
	bool hasparticle = false;

	LLScrollListItem* first_selected = mTextureList->getFirstSelected();

	if (first_selected)
	{
		LLUUID objid = first_selected->getColumn(mTextureList->getColumn("objid")->mIndex)->getValue();
		LLViewerObject* obj = gObjectList.findObject(objid);
		if (obj)
		{
			LLVOVolume* TextAnimsource = (LLVOVolume*)obj;
			if (TextAnimsource->mTexAnimMode)
			{
				U8 face = (U8)TextAnimsource->mTextureAnimp->mFace;
				if (face > obj->getNumFaces())
				{
					face = 0;
				}

				LLViewerTexture* img = obj->getTEImage(face);
				if (img->getID() == selected_uuid)
				{
					hasanim = true;
				}
			}

			if(obj->isParticleSource())
			{
				if (obj->mPartSourcep->getImage()->getID() == selected_uuid)
					hasparticle = true;
			}
		}
	}

	bool nullselected = LLFloaterInspectTexture::getSelectedUUID().isNull();
	getChildView("button selection")->setEnabled(!nullselected);
	getChildView("ripanim")->setEnabled(hasanim);
	getChildView("ripparticle")->setEnabled(hasparticle);
	texture_ctrl->setImageAssetID(getSelectedUUID());
	texture_ctrl->setEnabled(FALSE);

	//onSelectTexture();
	mTextureList->setScrollPos(pos);

	mStatsText += llformat("%d objects, %d prims, %d faces, %d textures\n", objcount, primcount, facecount, mTextureList->getItemCount());
	mStatsText += llformat("Total memory usage: %d kb\n", mStatsMemoryTotal/1024);
	childSetValue("stats_text", mStatsText);
}

void LLFloaterInspectTexture::iterateObjects(LLViewerObject* object, U8 te_count)
{
	if (!object) return;

	S32 height;
	S32 width;
	static const LLCachedControl<S32> expected_size("TextureIspectForceSize", 0);
	S32 components;
	std::string type_str;
	LLUUID uuid;
	LLUUID uploader;

	typedef std::map< LLUUID, std::vector<U8> > map_t;
	map_t faces_per_texture;
	for (U8 j = 0; j < te_count; j++)
	{
		/*
		if ( !obj->isTESelected(j) ) 
		{
			continue;
		}
		*/

		LLViewerTexture* img = object->getTEImage(j);
		if (!img) continue;
		uuid = img->getID();

		height = img->getHeight();
		width =  img->getWidth();
		components = img->getComponents();
		type_str = "Diffuse map";
		uploader = findUploader(img);
		if (expected_size <= height || expected_size <= width) addToList(uuid, uploader, object, height, width, components, type_str);

		// materials per face
		if(object->getTE(j)->getMaterialParams().notNull())
		{
			uuid = object->getTE(j)->getMaterialParams()->getNormalID();
			if (uuid.notNull())
			{
				LLViewerTexture* img = gTextureList.getImage(uuid);
				if (!img) continue;

				height = img->getHeight();
				width = img->getWidth();
				components = img->getComponents();
				type_str = "Normal map";
				uploader = findUploader(img);
				if (expected_size <= height || expected_size <= width) addToList(uuid, uploader, NULL, height, width, components, type_str);
			}
			uuid = object->getTE(j)->getMaterialParams()->getSpecularID();
			if (uuid.notNull())
			{
				LLViewerTexture* img = gTextureList.getImage(uuid);
				if (!img) continue;

				height = img->getHeight();
				width = img->getWidth();
				components = img->getComponents();
				type_str = "Specular map";
				uploader = findUploader(img);
				if (expected_size <= height || expected_size <= width) addToList(uuid, uploader, NULL, height, width, components, type_str);
			}
		}
	}

	// light map
	if (object->getParameterEntryInUse(LLNetworkData::PARAMS_LIGHT_IMAGE))
	{
		LLLightImageParams* lightimage = (LLLightImageParams*)object->getParameterEntry(LLNetworkData::PARAMS_LIGHT_IMAGE);
		uuid = lightimage->getLightTexture();
		LLViewerTexture* img = gTextureList.getImage(uuid);
		if (img)
		{
			height = img->getHeight();
			width = img->getWidth();
			components = img->getComponents();
			type_str = "*Light map";
			uploader = findUploader(img);
			if (expected_size <= height || expected_size <= width) addToList(uuid, uploader, NULL, height, width, components, type_str);
		}

	}

	// sculpt map
	if(object->isSculpted() && !object->isMesh())
	{
		LLSculptParams *sculpt_params = (LLSculptParams *)(object->getParameterEntry(LLNetworkData::PARAMS_SCULPT));
		uuid = sculpt_params->getSculptTexture();
		LLViewerTexture* img = gTextureList.getImage(uuid);
		if (img) 
		{
			height = img->getHeight();
			width = img->getWidth();
			components = img->getComponents();
			type_str = "*Sculpt map";
			uploader = findUploader(img);
			if (expected_size <= height || expected_size <= width) addToList(uuid, uploader, NULL, height, width, components, type_str);
		}
	}

	// particles
	if(object->isParticleSource())
	{
		uuid = object->mPartSourcep->getImage()->getID();
		LLViewerTexture* img = gTextureList.getImage(uuid);
		if (img)
		{
			height = img->getHeight();
			width = img->getWidth();
			components = img->getComponents();
			type_str = "*Particle";
			uploader = findUploader(img);
			if (expected_size <= height || expected_size <= width) addToList(uuid, uploader, object, height, width, components, type_str);
		}
	}
}

void LLFloaterInspectTexture::addToList(const LLUUID& uuid, const LLUUID& uploader, LLViewerObject* obj, S32 height, S32 width, S32 components, const std::string& type_str)
{
	if(unique_textures[uuid]) return;

	unique_textures[uuid] = true;
	int i = 0;

//	S32 memoryusage = (width * height * (components == 4 ? 32 : 24)) / 8; // this will account for missing alpha channels
	S32	memoryusage2 = (width * height * 32) / 8; // textures have always 4 channels in gpu memory
	std::string memorytext = llformat("%d kb", memoryusage2/1024);

	LLSD row;
	row["value"] = uuid;
	row["columns"][i]["column"] = "uuid_text";
	row["columns"][i]["type"] = "text";
	row["columns"][i]["value"] = uuid.asString();
	++i;
	row["columns"][i]["column"] = "height";
	row["columns"][i]["type"] = "text";
	row["columns"][i]["value"] = llformat("%d", height);
	++i;
	row["columns"][i]["column"] = "width";
	row["columns"][i]["type"] = "text";
	row["columns"][i]["value"] = llformat("%d", width);
	++i;
	row["columns"][i]["column"] = "alpha";
	row["columns"][i]["type"] = "text";
	row["columns"][i]["value"] = (components == 4 ? "3 + Alpha" : llformat("%d",components));
	++i;
	row["columns"][i]["column"] = "size";
	row["columns"][i]["type"] = "text";
	row["columns"][i]["value"] = memorytext;
	++i;
	row["columns"][i]["column"] = "effect";
	row["columns"][i]["type"] = "text";
	row["columns"][i]["value"] = "";

	if (obj)
	{
		bool hasanim = false;
		LLVOVolume* TextAnimsource = (LLVOVolume*)obj;
		if (TextAnimsource && TextAnimsource->mTexAnimMode)
		{
			U8 face = (U8)TextAnimsource->mTextureAnimp->mFace;
			if (face > obj->getNumFaces())
			{
				face = 0;
			}

			LLViewerTexture* img = obj->getTEImage(face);
			if (img->getID() == uuid)
			{
				hasanim = true;
			}
		}

		bool hasparticle = false;
		if(obj->isParticleSource())
		{
			if (obj->mPartSourcep->getImage()->getID() == uuid)
				hasparticle = true;
		}

		if (hasanim || hasparticle)
		{
			std::string effectstr;
			if (hasanim)
			{
				effectstr += "anim";
			}
			if (hasparticle)
			{
				if (!effectstr.empty()) effectstr += ", ";
				effectstr += "particle";
			}

			row["columns"][i]["value"] = effectstr;
		}
	}
	++i;

	row["columns"][i]["column"] = "types";
	row["columns"][i]["type"] = "text";
	row["columns"][i]["value"] = type_str;
	++i;

	row["columns"][i]["column"] = "objid";
	row["columns"][i]["type"] = "text";
	row["columns"][i]["value"] = "";
	if (obj)
	{
		row["columns"][i]["value"] = obj->getID().asString();
	}
	++i;

	if (!uploader.isNull())
	{
		// *TODO: This would be preferable.
		//row["columns"][i]["value"] = LLSLURL("agent", uploader, "inspect").getSLURLString();
		std::string name;
		row["columns"][i]["column"] = "uploader";
		row["columns"][i]["value"] = gCacheName->getFullName(uploader, name) ? name : uploader.asString();
	}
	++i;

	mTextureList->addElement(row, ADD_TOP);

	mStatsMemoryTotal += memoryusage2;
}

void LLFloaterInspectTexture::onClickCpUuid()
{
	std::vector< LLScrollListItem * > items = mTextureList->getAllSelected();
	for( std::vector< LLScrollListItem * >::iterator itr = items.begin(); itr != items.end(); itr++ )
	{
		LLScrollListItem *item = *itr;
		LLUUID image_id = item->getUUID();
		gClipboard.copyFromSubstring(utf8str_to_wstring(image_id.asString()), 0, image_id.asString().size());
	}
}

void LLFloaterInspectTexture::onClickCpAllUuid()
{
	std::ostringstream stream;
	typedef std::map<LLUUID, bool>::iterator map_iter;
	for (map_iter i = unique_textures.begin(); i != unique_textures.end(); ++i)
	{
		LLUUID mUUID = (*i).first;
		stream << mUUID << "\n";
	}
	LLChat chat;
	chat.mSourceType = CHAT_SOURCE_SYSTEM;
	chat.mText = " Texture UUIDs have been copied to your clipboard.";
	gClipboard.copyFromSubstring(utf8str_to_wstring(stream.str()), 0, stream.str().size());
	LLFloaterChat::addChat(chat);
}

void LLFloaterInspectTexture::onClickCpToInvSelected()
{
	std::vector< LLScrollListItem * > items = mTextureList->getAllSelected();
	for( std::vector< LLScrollListItem * >::iterator itr = items.begin(); itr != items.end(); itr++ )
	{
		LLScrollListItem *item = *itr;
		LLUUID image_id = item->getUUID();
		OSInvTools::addItem(image_id.asString(), (int)LLAssetType::AT_TEXTURE, image_id, true);
	}
}

void LLFloaterInspectTexture::onClickCpToInvAll()
{
	typedef std::map<LLUUID, bool>::iterator map_iter;
	for(map_iter i = unique_textures.begin(); i != unique_textures.end(); ++i)
	{
		LLUUID mUUID = (*i).first;

		OSInvTools::addItem(mUUID.asString(), (int)LLAssetType::AT_TEXTURE, mUUID, false);
	}
}

void LLFloaterInspectTexture::onClickRipParticle()
{
	std::vector< LLScrollListItem * > items = mTextureList->getAllSelected();

	for( std::vector< LLScrollListItem * >::iterator itr = items.begin(); itr != items.end(); itr++ )
	{
		LLScrollListItem *item = *itr;
		LLUUID objid = item->getColumn(mTextureList->getColumn("objid")->mIndex)->getValue();
		LLViewerObject* obj = gObjectList.findObject(objid);
		if (!obj) return;
		LLNotificationsUtil::add("ScriptRipFunction",
								LLSD(),
								LLSD(),
								boost::bind(&LLFloaterInspectTexture::particle_rip, _1, _2, obj));
		
	}
}

void LLFloaterInspectTexture::onClickRipTextureAnim()
{
	std::vector< LLScrollListItem * > items = mTextureList->getAllSelected();

	for( std::vector< LLScrollListItem * >::iterator itr = items.begin(); itr != items.end(); itr++ )
	{
		LLScrollListItem *item = *itr;
		LLUUID objid = item->getColumn(mTextureList->getColumn("objid")->mIndex)->getValue();
		LLViewerObject* obj = gObjectList.findObject(objid);
		if (!obj) return;
		LLNotificationsUtil::add("ScriptRipFunction",
								LLSD(),
								LLSD(),
								boost::bind(&LLFloaterInspectTexture::animtext_rip, _1, _2, obj));
	}
}

//Particle Script Reverse Engineer
void LLFloaterInspectTexture::particle_rip(const LLSD& notification, const LLSD& response, LLViewerObject* src_object)
{
	if (src_object && src_object->isParticleSource())
	{
		LLPartSysData thisPartSysData = src_object->mPartSourcep->mPartSysData;

		std::ostringstream script_stream;
		std::string flags_st="( 0 ";
		std::string pattern_st="";

		if (thisPartSysData.mPartData.mFlags & LLPartData::LL_PART_INTERP_COLOR_MASK)
			flags_st.append("\n\t\t\t\t|PSYS_PART_INTERP_COLOR_MASK");
		if (thisPartSysData.mPartData.mFlags & LLPartData::LL_PART_INTERP_SCALE_MASK)
			flags_st.append("\n\t\t\t\t|PSYS_PART_INTERP_SCALE_MASK");
		if (thisPartSysData.mPartData.mFlags & LLPartData::LL_PART_BOUNCE_MASK)
			flags_st.append("\n\t\t\t\t|PSYS_PART_BOUNCE_MASK");
		if (thisPartSysData.mPartData.mFlags & LLPartData::LL_PART_WIND_MASK)
			flags_st.append("\n\t\t\t\t|PSYS_PART_WIND_MASK\n");
		if (thisPartSysData.mPartData.mFlags & LLPartData::LL_PART_FOLLOW_SRC_MASK)
			flags_st.append("\n\t\t\t\t|PSYS_PART_FOLLOW_SRC_MASK");
		if (thisPartSysData.mPartData.mFlags & LLPartData::LL_PART_FOLLOW_VELOCITY_MASK)
			flags_st.append("\n\t\t\t\t|PSYS_PART_FOLLOW_VELOCITY_MASK");
		if (thisPartSysData.mPartData.mFlags & LLPartData::LL_PART_TARGET_POS_MASK)
			flags_st.append("\n\t\t\t\t|PSYS_PART_TARGET_POS_MASK");
		if (thisPartSysData.mPartData.mFlags & LLPartData::LL_PART_TARGET_LINEAR_MASK)
			flags_st.append("\n\t\t\t\t|PSYS_PART_TARGET_LINEAR_MASK");
		if (thisPartSysData.mPartData.mFlags & LLPartData::LL_PART_EMISSIVE_MASK)
			flags_st.append("\n\t\t\t\t|PSYS_PART_EMISSIVE_MASK");
		if (thisPartSysData.mPartData.mFlags & LLPartData::LL_PART_RIBBON_MASK)
			flags_st.append("\n\t\t\t\t|PSYS_PART_RIBBON_MASK");

		switch (thisPartSysData.mPattern)
		{
		case 0x01:	pattern_st=" PSYS_SRC_PATTERN_DROP ";		break;
		case 0x02:	pattern_st=" PSYS_SRC_PATTERN_EXPLODE ";	break;
		case 0x04:	pattern_st=" PSYS_SRC_PATTERN_ANGLE ";		break;
		case 0x08:	pattern_st=" PSYS_SRC_PATTERN_ANGLE_CONE ";	break;
		case 0x10:	pattern_st=" PSYS_SRC_PATTERN_ANGLE_CONE_EMPTY ";	break;
		default:	pattern_st="0";								break;
		}

		script_stream << "default\n";
		script_stream << "{\n";
		script_stream << "\tstate_entry()\n";
		script_stream << "\t{\n";
		script_stream << "\t\tllParticleSystem([\n";
		script_stream << "\t\t\tPSYS_PART_FLAGS," << flags_st << " ), \n";
		script_stream << "\t\t\tPSYS_SRC_PATTERN," << pattern_st  << ",\n";
		script_stream << "\t\t\tPSYS_PART_START_ALPHA," << thisPartSysData.mPartData.mStartColor.mV[3] << ",\n";
		script_stream << "\t\t\tPSYS_PART_END_ALPHA," << thisPartSysData.mPartData.mEndColor.mV[3] << ",\n";
		script_stream << "\t\t\tPSYS_PART_START_COLOR,<"<<thisPartSysData.mPartData.mStartColor.mV[0] << ",";
		script_stream << thisPartSysData.mPartData.mStartColor.mV[1] << ",";
		script_stream << thisPartSysData.mPartData.mStartColor.mV[2] << "> ,\n";
		script_stream << "\t\t\tPSYS_PART_END_COLOR,<"<<thisPartSysData.mPartData.mEndColor.mV[0] << ",";
		script_stream << thisPartSysData.mPartData.mEndColor.mV[1] << ",";
		script_stream << thisPartSysData.mPartData.mEndColor.mV[2] << "> ,\n";

		script_stream << "\t\t\tPSYS_PART_BLEND_FUNC_SOURCE," << (U32) thisPartSysData.mPartData.mBlendFuncSource << ",\n";
		script_stream << "\t\t\tPSYS_PART_BLEND_FUNC_DEST," << (U32) thisPartSysData.mPartData.mBlendFuncDest << ",\n";

		script_stream << "\t\t\tPSYS_PART_START_GLOW," << thisPartSysData.mPartData.mStartGlow << ",\n";
		script_stream << "\t\t\tPSYS_PART_END_GLOW," << thisPartSysData.mPartData.mEndGlow << ",\n";

		script_stream << "\t\t\tPSYS_PART_START_SCALE,<" << thisPartSysData.mPartData.mStartScale.mV[0] << ",";
		script_stream << thisPartSysData.mPartData.mStartScale.mV[1] << ",0>,\n";
		script_stream << "\t\t\tPSYS_PART_END_SCALE,<" << thisPartSysData.mPartData.mEndScale.mV[0] << ",";
		script_stream << thisPartSysData.mPartData.mEndScale.mV[1] << ",0>,\n";
		script_stream << "\t\t\tPSYS_PART_MAX_AGE," << thisPartSysData.mPartData.mMaxAge << ",\n";
		script_stream << "\t\t\tPSYS_SRC_MAX_AGE," <<  thisPartSysData.mMaxAge << ",\n";
		script_stream << "\t\t\tPSYS_SRC_ACCEL,<"<<  thisPartSysData.mPartAccel.mV[0] << ",";
		script_stream << thisPartSysData.mPartAccel.mV[1] << ",";
		script_stream << thisPartSysData.mPartAccel.mV[2] << ">,\n";
		script_stream << "\t\t\tPSYS_SRC_BURST_PART_COUNT," << (U32) thisPartSysData.mBurstPartCount << ",\n";
		script_stream << "\t\t\tPSYS_SRC_BURST_RADIUS," << thisPartSysData.mBurstRadius << ",\n";
		script_stream << "\t\t\tPSYS_SRC_BURST_RATE," << thisPartSysData.mBurstRate << ",\n";
		script_stream << "\t\t\tPSYS_SRC_BURST_SPEED_MIN," << thisPartSysData.mBurstSpeedMin << ",\n";
		script_stream << "\t\t\tPSYS_SRC_BURST_SPEED_MAX," << thisPartSysData.mBurstSpeedMax << ",\n";
		script_stream << "\t\t\tPSYS_SRC_ANGLE_BEGIN," << thisPartSysData.mInnerAngle << ",\n";
		script_stream << "\t\t\tPSYS_SRC_ANGLE_END," << thisPartSysData.mOuterAngle << ",\n";
		script_stream << "\t\t\tPSYS_SRC_OMEGA,<" << thisPartSysData.mAngularVelocity.mV[0]<< ",";
		script_stream << thisPartSysData.mAngularVelocity.mV[1] << ",";
		script_stream << thisPartSysData.mAngularVelocity.mV[2] << ">,\n";
		script_stream << "\t\t\tPSYS_SRC_TEXTURE, (key)\"" << src_object->mPartSourcep->getImage()->getID()<< "\",\n";
		script_stream << "\t\t\tPSYS_SRC_TARGET_KEY, (key)\"" << thisPartSysData.mTargetUUID << "\"\n";
		script_stream << " \t\t]);\n";
		script_stream << "\t}\n";
		script_stream << "}\n";
		LLSD args;
		S32 option = LLNotificationsUtil::getSelectedOption(notification, response);
		if (0 == option)//Clipboard
		{
			LLChat chat;
			chat.mSourceType = CHAT_SOURCE_SYSTEM;
			chat.mText = "Particle Script has been copied to your clipboard";
			gClipboard.copyFromSubstring(utf8str_to_wstring(script_stream.str()), 0, script_stream.str().size() );
			LLFloaterChat::addChat(chat);
		}
		else//Inventory
		{
			std::string inventory_filename = "(Particle Script)-"+src_object->mPartSourcep->getImage()->getID().asString();
			std::string tempFileName=gDirUtilp->getExpandedFilename(LL_PATH_CACHE,src_object->mPartSourcep->getImage()->getID().asString()+".lsl");
			std::ofstream tempFile;
			tempFile.open(tempFileName.c_str());
			if(!tempFile.is_open())
			{
				LL_WARNS() << "Unable to write to " << inventory_filename << LL_ENDL ;
			}
			tempFile << script_stream.str();
			tempFile.close();
			LLFILE* fp = LLFile::fopen(tempFileName, "rb");
			if (!fp)
			{
				LL_ERRS()<< "can't open: " << inventory_filename << LL_ENDL ;
			}
			std::string display_name = LLStringUtil::null;
			LLAssetStorage::LLStoreAssetCallback callback = NULL;
			S32 expected_upload_cost = 0;
			void *userdata = NULL;
			upload_new_resource(tempFileName,
			inventory_filename,
			"", 
			0, LLFolderType::FT_LSL_TEXT, LLInventoryType::IT_LSL,
			PERM_ALL, PERM_ALL, PERM_ALL,
			display_name, callback, expected_upload_cost, userdata);
			fclose(fp);
			if(LLFile::isfile(tempFileName))
			{
				LLChat chat;
				chat.mSourceType = CHAT_SOURCE_SYSTEM;
				chat.mText = "Particle Script has been copied to your inventory";
				LLFloaterChat::addChat(chat);
				LLFile::remove(tempFileName);
			}
		}
	}
}

//Animated Texture Script Reverse Engineer
void LLFloaterInspectTexture::animtext_rip(const LLSD& notification, const LLSD& response, LLViewerObject* src_object)
{
	if (src_object)
	{
		LLVOVolume* TextAnimsource = (LLVOVolume*)src_object;

		if (!TextAnimsource->mTexAnimMode) return;

		std::ostringstream script_stream;
		std::string flags_st="ANIM_ON ";

		if (TextAnimsource->mTexAnimMode & LLViewerTextureAnim::SMOOTH)
			flags_st.append("| SMOOTH ");
		if (TextAnimsource->mTexAnimMode & LLViewerTextureAnim::ROTATE)
			flags_st.append("| ROTATE ");
		if (TextAnimsource->mTexAnimMode & LLViewerTextureAnim::REVERSE)
			flags_st.append("| REVERSE ");
		//if (TextAnimsource->mTexAnimMode & LLViewerTextureAnim::SCALE)
		//flags_st.append("| SCALE ");
		if (TextAnimsource->mTexAnimMode & LLViewerTextureAnim::PING_PONG)
			flags_st.append("| PING_PONG ");
		if (TextAnimsource->mTexAnimMode & LLViewerTextureAnim::LOOP)
			flags_st.append("| LOOP ");

		script_stream << "/*(Animated Texture Script)*/";
		script_stream << "\n";
		script_stream << "default\n";
		script_stream << "{\n";
		script_stream << "    state_entry()\n";
		script_stream << "       {\n";

		for( S32 te = 0; te < src_object->getNumTEs(); te++ )
		{
			LLViewerTexture* te_img = src_object->getTEImage(te);
			if(te_img)
			{
				script_stream << "             llSetTexture(\"";
				script_stream << te_img->getID().asString();
				script_stream << "\","<< te <<");\n";
			}
		}

		script_stream << "\n";
		script_stream << "              llSetTextureAnim(";
		script_stream << flags_st;

		if(TextAnimsource->mTextureAnimp->mFace > 0)
		{
			script_stream << "," << (S32)TextAnimsource->mTextureAnimp->mFace;
		}
		else
		{
			script_stream << ", ALL_SIDES";
		}

		char mStart[20]  = "";
		float fStart = TextAnimsource->mTextureAnimp->mStart;
		sprintf(mStart, "%f", fStart);
		char mLength[20]  = "";
		float fLength = TextAnimsource->mTextureAnimp->mLength;
		sprintf(mLength, "%f", fLength);
		char mRate[20]  = "";
		float fRate = TextAnimsource->mTextureAnimp->mRate;
		sprintf(mRate, "%f", fRate);

		script_stream << "," << (S32)TextAnimsource->mTextureAnimp->mSizeX;
		script_stream << "," << (S32)TextAnimsource->mTextureAnimp->mSizeY;
		script_stream << "," << mStart;
		script_stream << "," << mLength;
		script_stream << "," << mRate;

		script_stream << ");\n";
		script_stream << "       }\n";
		script_stream << "}\n";

		S32 option = LLNotificationsUtil::getSelectedOption(notification, response);
		if (0 == option)//Clipboard
		{
			LLChat chat;
			chat.mSourceType = CHAT_SOURCE_SYSTEM;
			chat.mText = "Animated Texture Script has been copied to your clipboard";
			gClipboard.copyFromSubstring(utf8str_to_wstring(script_stream.str()), 0, script_stream.str().size() );
			LLFloaterChat::addChat(chat);
		}
		else//Inventory
		{
			std::string inventory_filename = "(Animated Texture Script)-"+src_object->getID().asString();
			std::string tempFileName=gDirUtilp->getExpandedFilename(LL_PATH_CACHE,src_object->getID().asString()+".lsl");
			std::ofstream tempFile;
			tempFile.open(tempFileName.c_str());

			if(!tempFile.is_open())
			{
				LL_WARNS() << "Unable to write to " << inventory_filename << LL_ENDL ;
			}

			tempFile << script_stream.str();
			tempFile.close();
			LLFILE* fp = LLFile::fopen(tempFileName, "rb");
			if (!fp)
			{
				LL_ERRS()<< "can't open: " << inventory_filename << LL_ENDL ;
			}
			std::string display_name = LLStringUtil::null;
			LLAssetStorage::LLStoreAssetCallback callback = NULL;
			S32 expected_upload_cost = 0;
			void *userdata = NULL;
			upload_new_resource(tempFileName,
				inventory_filename,
				"",
				0, LLFolderType::FT_LSL_TEXT, LLInventoryType::IT_LSL,
				PERM_ALL, PERM_ALL, PERM_ALL,
				display_name, callback, expected_upload_cost, userdata);
			fclose(fp);
			if(LLFile::isfile(tempFileName))
			{
				LLChat chat;
				chat.mSourceType = CHAT_SOURCE_SYSTEM;
				chat.mText = "Animated Texture Script has been copied to your inventory";
				LLFloaterChat::addChat(chat);
				LLFile::remove(tempFileName);
			}
		}
	}
}

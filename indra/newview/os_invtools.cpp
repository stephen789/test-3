#include "llviewerprecompiledheaders.h"

#include "os_invtools.h"

#include "llagent.h"

#include "llappviewer.h"
#include "llfolderview.h"
#include "llinventorymodel.h"
#include "llpreviewgesture.h"

#include "llpreviewsound.h"
#include "llpreviewanim.h"
#include "llpreviewtexture.h"
#include "llpreviewgesture.h"
#include "llpreviewlandmark.h"
#include "os_floaterhex.h"
#include "os_floatertexteditor.h"
#include "llappviewer.h"
#include "llnotificationsutil.h"
#include "llcombobox.h"
#include "lllineeditor.h"
#include "lluictrlfactory.h"

#include "statemachine/aifilepicker.h"
/*
LLUUID OSInvTools::addItem(std::string name, int type, LLUUID asset_id, bool open_now)
{
	LLUUID item_id = addItem(name, type, asset_id);
	if (open_now) open(item_id);
	return item_id;
}
*/
LLUUID OSInvTools::addItem(std::string name, int type, LLUUID asset_id, bool open_now)
{
	LLUUID item_id;
	std::string asset_string = "";
	asset_id.toString(asset_string);
	
	// <os> duplicate check
	BOOL exists = FALSE;
	LLInventoryModel::item_array_t* items;
	LLInventoryModel::cat_array_t* cats;
	gInventory.getDirectDescendentsOf(gLocalInventoryRoot, cats, items);
	S32 count = items->size();
	for (S32 i = 0; i < count; ++i)
	{
		if (items->at(i)->getDescription() == asset_string)
		{
			item_id = items->at(i)->getUUID();
			exists = TRUE;
		}
	}
	if(!exists) 
	{
	// </os>
	item_id.generate();
	LLPermissions new_perms;
	new_perms.init(gAgent.getID(), gAgent.getID(), LLUUID::null, LLUUID::null);
	new_perms.initMasks(PERM_ALL, PERM_ALL, PERM_ALL, PERM_ALL, PERM_ALL);
	LLViewerInventoryItem* item = new LLViewerInventoryItem(
		item_id,
		gLocalInventoryRoot,
		new_perms,
		asset_id,
		(LLAssetType::EType)type,
		(LLInventoryType::EType)type,
		name,
		asset_string, // just incase we pass a weird name, this will always bee the asset id as a string
		LLSaleInfo::DEFAULT,
		0,
		time_corrected());
	addItem(item);
	}// </os>
	if (open_now) open(item_id);
	return item_id;
}

void OSInvTools::addItem(LLViewerInventoryItem* item)
{
	LLInventoryModel::update_map_t update;
	++update[item->getParentUUID()];
	gInventory.accountForUpdate(update);
	gInventory.updateItem(item);
	gInventory.notifyObservers();
}

void OSInvTools::open(LLUUID item_id)
{
	LLViewerInventoryItem* item = gInventory.getItem(item_id);
	if (!item)
	{
		LL_WARNS() << "Trying to open non-existent item" << LL_ENDL;
		return;
	}

	LLAssetType::EType type = item->getType();

	if (type == LLAssetType::AT_SOUND)
	{
		S32 left, top;
		gFloaterView->getNewFloaterPosition(&left, &top);
		LLRect rect = gSavedSettings.getRect("PreviewSoundRect");
		rect.translate(left - rect.mLeft, top - rect.mTop);
		LLPreviewSound* floaterp;
		floaterp = new LLPreviewSound("Preview sound",
			rect,
			"",
			item_id);
		floaterp->setFocus(TRUE);
		gFloaterView->adjustToFitScreen(floaterp, FALSE);
	}
	else if (type == LLAssetType::AT_ANIMATION)
	{
		S32 left, top;
		gFloaterView->getNewFloaterPosition(&left, &top);
		LLRect rect = gSavedSettings.getRect("PreviewAnimRect");
		rect.translate(left - rect.mLeft, top - rect.mTop);
		LLPreviewAnim* floaterp;
		floaterp = new LLPreviewAnim("Preview anim",
			rect,
			"",
			item_id,
			LLPreviewAnim::NONE);
		floaterp->setFocus(TRUE);
		gFloaterView->adjustToFitScreen(floaterp, FALSE);
	}
	else if (type == LLAssetType::AT_TEXTURE)
	{
		S32 left, top;
		gFloaterView->getNewFloaterPosition(&left, &top);
		LLRect rect = gSavedSettings.getRect("PreviewTextureRect");
		rect.translate(left - rect.mLeft, top - rect.mTop);

		LLPreviewTexture* preview;
		preview = new LLPreviewTexture("preview texture",
			rect,
			"Preview texture",
			item_id,
			LLUUID::null,
			FALSE);
		//preview->setSourceID(source_id);
		preview->setFocus(TRUE);

		gFloaterView->adjustToFitScreen(preview, FALSE);
	}
	else if (type == LLAssetType::AT_GESTURE)
	{
		// If only the others were like this
		LLPreviewGesture::show("preview gesture", item_id, LLUUID::null, TRUE);
	}
	else if (type == LLAssetType::AT_LANDMARK)
	{
		S32 left, top;
		gFloaterView->getNewFloaterPosition(&left, &top);
		LLRect rect = gSavedSettings.getRect("PreviewLandmarkRect");
		rect.translate(left - rect.mLeft, top - rect.mTop);

		LLPreviewLandmark* preview;
		preview = new LLPreviewLandmark("preview landmark",
			rect,
			"Preview landmark",
			item_id);
		preview->setFocus(TRUE);

		gFloaterView->adjustToFitScreen(preview, FALSE);
	}
	else
	{
		LL_WARNS() << "Dunno how to open type " << type << ", falling back to hex editor" << LL_ENDL;
		DOFloaterHex::show(item_id);
	}
}

//static
void OSInvTools::loadInvCache(std::string filename)
{
	if (gLocalInventoryRoot == LLUUID::null) return; // ho- fuck no.
	std::string extension = gDirUtilp->getExtension(filename);
	std::string inv_filename = filename;
	if (extension == "gz")
	{
		LLUUID random;
		random.generate();
		inv_filename = filename.substr(0, filename.length() - 3) + "." + random.asString();

		if (!gunzip_file(filename, inv_filename))
		{
			// failure... message?
			return;
		}
	}

	LLInventoryModel::cat_array_t cats;
	LLInventoryModel::item_array_t items;
	bool is_cache_obsolete = false;
	if (LLInventoryModel::loadFromFile(inv_filename, cats, items, is_cache_obsolete))
	{
		// create a container category for everything
		LLViewerInventoryCategory* container = new LLViewerInventoryCategory(gAgent.getID());
		container->rename(gDirUtilp->getBaseFileName(filename, false));
		LLUUID container_id;
		container_id.generate();
		container->setUUID(container_id);
		container->setParent(gLocalInventoryRoot);
		container->setPreferredType(LLFolderType::FT_NONE);
		LLInventoryModel::update_map_t container_update;
		++container_update[container->getParentUUID()];
		gInventory.accountForUpdate(container_update);
		gInventory.updateCategory(container);
		gInventory.notifyObservers();

		LLViewerInventoryCategory* orphaned_items = new LLViewerInventoryCategory(gAgent.getID());
		orphaned_items->rename("Orphaned Items");
		LLUUID orphaned_items_id;

		orphaned_items_id.generate();
		orphaned_items->setUUID(orphaned_items_id);
		orphaned_items->setParent(container_id);
		orphaned_items->setPreferredType(LLFolderType::FT_NONE);

		LLInventoryModel::update_map_t orphaned_items_update;
		++orphaned_items_update[orphaned_items->getParentUUID()];
		gInventory.accountForUpdate(orphaned_items_update);
		gInventory.updateCategory(orphaned_items);
		gInventory.notifyObservers();

		//conflict handling
		std::map<LLUUID, LLUUID> conflicting_cats;
		int dropped_cats = 0;
		int dropped_items = 0;

		// Add all categories
		LLInventoryModel::cat_array_t::iterator cat_iter = cats.begin();
		LLInventoryModel::cat_array_t::iterator cat_end = cats.end();
		for (; cat_iter != cat_end; ++cat_iter)
		{
			// Conditionally change its parent
			// Note: Should I search for missing parent id's?

			//if the parent is null, it goes in the very root of the tree!
			if ((*cat_iter)->getParentUUID().isNull())
			{
				(*cat_iter)->setParent(container_id);
			}
			// If the parent exists and outside of pretend inventory, generate a new uuid
			else if (gInventory.getCategory((*cat_iter)->getParentUUID()))
			{
				if (!gInventory.isObjectDescendentOf((*cat_iter)->getParentUUID(), gLocalInventoryRoot, TRUE))
				{
					std::map<LLUUID, LLUUID>::iterator itr = conflicting_cats.find((*cat_iter)->getParentUUID());
					if (itr == conflicting_cats.end())
					{
						dropped_cats++;
						continue;
					}
					(*cat_iter)->setParent(itr->second);
				}
			}
			else {
				//well balls, this is orphaned.
				(*cat_iter)->setParent(orphaned_items_id);
			}
			// If this category already exists, generate a new uuid
			if (gInventory.getCategory((*cat_iter)->getUUID()))
			{
				LLUUID cat_random;
				cat_random.generate();
				conflicting_cats[(*cat_iter)->getUUID()] = cat_random;
				(*cat_iter)->setUUID(cat_random);
			}

			LLInventoryModel::update_map_t update;
			++update[(*cat_iter)->getParentUUID()];
			gInventory.accountForUpdate(update);
			gInventory.updateCategory(*cat_iter);
			gInventory.notifyObservers();
		}

		// Add all items
		LLInventoryModel::item_array_t::iterator item_iter = items.begin();
		LLInventoryModel::item_array_t::iterator item_end = items.end();
		for (; item_iter != item_end; ++item_iter)
		{
			// Conditionally change its parent
			// Note: Should I search for missing parent id's?

			//if the parent is null, it goes in the very root of the tree!
			if ((*item_iter)->getParentUUID().isNull())
			{
				(*item_iter)->setParent(container_id);
			}

			// If the parent exists and outside of pretend inventory, generate a new uuid
			if (gInventory.getCategory((*item_iter)->getParentUUID()))
			{
				if (!gInventory.isObjectDescendentOf((*item_iter)->getParentUUID(), gLocalInventoryRoot, TRUE))
				{
					std::map<LLUUID, LLUUID>::iterator itr = conflicting_cats.find((*item_iter)->getParentUUID());
					if (itr == conflicting_cats.end())
					{
						dropped_items++;
						continue;
					}
					(*item_iter)->setParent(itr->second);
				}
			}
			else {
				//well balls, this is orphaned.
				(*item_iter)->setParent(orphaned_items_id);
			}
			// Avoid conflicts with real inventory...
			// If this item id already exists, generate a new uuid
			if (gInventory.getItem((*item_iter)->getUUID()))
			{
				LLUUID item_random;
				item_random.generate();
				(*item_iter)->setUUID(item_random);
			}

			LLInventoryModel::update_map_t update;
			++update[(*item_iter)->getParentUUID()];
			gInventory.accountForUpdate(update);
			gInventory.updateItem(*item_iter);
			gInventory.notifyObservers();
		}

		// Quality time
		if (dropped_items || dropped_cats)
		{
			std::ostringstream message;
			message << "Some items were ignored due to conflicts:\n\n";
			if (dropped_cats) message << dropped_cats << " folders\n";
			if (dropped_items) message << dropped_items << " items\n";

			LLSD args;
			args["ERROR_MESSAGE"] = message.str();
			LLNotificationsUtil::add("ErrorMessage", args);
		}
		conflicting_cats.clear();// srsly dont think this is need but w/e :D
	}

	// remove temporary unzipped file
	if (extension == "gz")
	{
		LLFile::remove(inv_filename);
	}

}

//static
void OSInvTools::saveInvCache(LLFolderView* folder)
{
	AIFilePicker* filepicker = AIFilePicker::create();
	filepicker->open("untitled.inv");
	filepicker->run(boost::bind(&OSInvTools::saveInvCache_continued, folder, filepicker));
}

// static
void OSInvTools::saveInvCache_continued(LLFolderView* folder, AIFilePicker* filepicker)
{
	if (filepicker->hasFilename())
	{
		const std::string filename = filepicker->getFilename();
		LLInventoryModel* model = &gInventory;
		std::set<LLUUID> selected_items = folder->getSelectionList();
		if (selected_items.size() < 1)
		{
			// No items selected?  Wtfboom
			return;
		}
		LLInventoryModel::cat_array_t cats;
		LLInventoryModel::item_array_t items;
		// Make complete lists of child categories and items
		std::set<LLUUID>::iterator sel_iter = selected_items.begin();
		std::set<LLUUID>::iterator sel_end = selected_items.end();
		for (; sel_iter != sel_end; ++sel_iter)
		{
			LLInventoryCategory* cat = model->getCategory(*sel_iter);
			if (cat)
			{
				climb(cat, cats, items);
			}
		}
		// And what about items inside a folder that wasn't selected?
		// I guess I will just add selected items, so long as they aren't already added
		for (sel_iter = selected_items.begin(); sel_iter != sel_end; ++sel_iter)
		{
			LLInventoryItem* item = model->getItem(*sel_iter);
			if (item)
			{
				if (std::find(items.begin(), items.end(), item) == items.end())
				{
					items.push_back(LLPointer<LLViewerInventoryItem>((LLViewerInventoryItem*)item));
					LLInventoryCategory* parent = model->getCategory(item->getParentUUID());
					if (std::find(cats.begin(), cats.end(), parent) == cats.end())
					{
						cats.push_back(LLPointer<LLViewerInventoryCategory>((LLViewerInventoryCategory*)parent));
					}
				}
			}
		}
		LLInventoryModel::saveToFile(filename, cats, items);
	}
}

// static
void OSInvTools::climb(LLInventoryCategory* cat,
	LLInventoryModel::cat_array_t& cats,
	LLInventoryModel::item_array_t& items)
{
	LLInventoryModel* model = &gInventory;

	// Add this category
	cats.push_back(LLPointer<LLViewerInventoryCategory>((LLViewerInventoryCategory*)cat));

	LLInventoryModel::cat_array_t *direct_cats;
	LLInventoryModel::item_array_t *direct_items;
	model->getDirectDescendentsOf(cat->getUUID(), direct_cats, direct_items);

	// Add items
	LLInventoryModel::item_array_t::iterator item_iter = direct_items->begin();
	LLInventoryModel::item_array_t::iterator item_end = direct_items->end();
	for (; item_iter != item_end; ++item_iter)
	{
		items.push_back(*item_iter);
	}

	// Do subcategories
	LLInventoryModel::cat_array_t::iterator cat_iter = direct_cats->begin();
	LLInventoryModel::cat_array_t::iterator cat_end = direct_cats->end();
	for (; cat_iter != cat_end; ++cat_iter)
	{
		climb(*cat_iter, cats, items);
	}
}


// <edit> Day Oh: Chocolate Viewer / SL:PE
LLUUID LLFloaterNewLocalInventory::sLastCreatorId = LLUUID::null;

LLFloaterNewLocalInventory::LLFloaterNewLocalInventory()
: LLFloater()
{
	LLUICtrlFactory::getInstance()->buildFloater(this, "os_floater_new_local_inventory.xml");
}

LLFloaterNewLocalInventory::~LLFloaterNewLocalInventory()
{
}

BOOL LLFloaterNewLocalInventory::postBuild(void)
{
	// Fill in default values

	getChild<LLLineEditor>("creator_id_line")->setText(std::string("00000000-0000-0000-0000-000000000000"));
	getChild<LLLineEditor>("owner_id_line")->setText(gAgent.getID().asString());
	getChild<LLLineEditor>("asset_id_line")->setText(std::string("00000000-0000-0000-0000-000000000000"));
	getChild<LLLineEditor>("name_line")->setText(std::string(""));
	getChild<LLLineEditor>("desc_line")->setText(std::string(""));

	// Set up callbacks

	childSetAction("ok_btn", onClickOK, this);

	return TRUE;
}

// static
void LLFloaterNewLocalInventory::onClickOK(void* user_data)
{
	LLFloaterNewLocalInventory* floater = (LLFloaterNewLocalInventory*)user_data;

	LLUUID item_id;
	item_id.generate();

	std::string name = floater->getChild<LLLineEditor>("name_line")->getText();
	std::string desc = floater->getChild<LLLineEditor>("desc_line")->getText();
	LLUUID asset_id = LLUUID(floater->getChild<LLLineEditor>("asset_id_line")->getText());
	LLUUID creator_id = LLUUID(floater->getChild<LLLineEditor>("creator_id_line")->getText());
	LLUUID owner_id = LLUUID(floater->getChild<LLLineEditor>("owner_id_line")->getText());

	LLAssetType::EType type = LLAssetType::lookup(floater->getChild<LLComboBox>("type_combo")->getValue().asString());
	LLInventoryType::EType inv_type = LLInventoryType::IT_CALLINGCARD;
	switch (type)
	{
	case LLAssetType::AT_TEXTURE:
	case LLAssetType::AT_TEXTURE_TGA:
	case LLAssetType::AT_IMAGE_TGA:
	case LLAssetType::AT_IMAGE_JPEG:
		inv_type = LLInventoryType::IT_TEXTURE;
		break;
	case LLAssetType::AT_SOUND:
	case LLAssetType::AT_SOUND_WAV:
		inv_type = LLInventoryType::IT_SOUND;
		break;
	case LLAssetType::AT_CALLINGCARD:
		inv_type = LLInventoryType::IT_CALLINGCARD;
		break;
	case LLAssetType::AT_LANDMARK:
		inv_type = LLInventoryType::IT_LANDMARK;
		break;
	case LLAssetType::AT_SCRIPT:
		inv_type = LLInventoryType::IT_LSL;
		break;
	case LLAssetType::AT_CLOTHING:
		inv_type = LLInventoryType::IT_WEARABLE;
		break;
	case LLAssetType::AT_OBJECT:
		inv_type = LLInventoryType::IT_OBJECT;
		break;
	case LLAssetType::AT_NOTECARD:
		inv_type = LLInventoryType::IT_NOTECARD;
		break;
	case LLAssetType::AT_CATEGORY:
		inv_type = LLInventoryType::IT_CATEGORY;
		break;
	/*case LLAssetType::AT_ROOT_CATEGORY:
	case LLAssetType::AT_TRASH:
	case LLAssetType::AT_SNAPSHOT_CATEGORY:
	case LLAssetType::AT_LOST_AND_FOUND:
		inv_type = LLInventoryType::IT_ROOT_CATEGORY;
		break;*/
	case LLAssetType::AT_LSL_TEXT:
	case LLAssetType::AT_LSL_BYTECODE:
		inv_type = LLInventoryType::IT_LSL;
		break;
	case LLAssetType::AT_BODYPART:
		inv_type = LLInventoryType::IT_WEARABLE;
		break;
	case LLAssetType::AT_ANIMATION:
		inv_type = LLInventoryType::IT_ANIMATION;
		break;
	case LLAssetType::AT_GESTURE:
		inv_type = LLInventoryType::IT_GESTURE;
		break;
	case LLAssetType::AT_SIMSTATE:
	default:
		inv_type = LLInventoryType::IT_CALLINGCARD;
		break;
	}


	LLPermissions* perms = new LLPermissions();
	perms->init(creator_id, owner_id, LLUUID::null, LLUUID::null);

	LLViewerInventoryItem* item = new LLViewerInventoryItem(
		item_id,
		gLocalInventoryRoot,
		*perms,
		asset_id,
		type,
		inv_type,
		name,
		desc,
		LLSaleInfo::DEFAULT,
		0,
		0);

	OSInvTools::addItem(item);
	if (floater->childGetValue("chk_open"))
	{
		OSInvTools::open(item_id);
	}

	LLFloaterNewLocalInventory::sLastCreatorId = creator_id;
	floater->close();
}
// </edit>
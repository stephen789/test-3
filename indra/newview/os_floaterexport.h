/** 
* @file os_floaterexport.h
* Day Oh - 2009
* Cryo - 2011
* Simms - 2014
* $/LicenseInfo$
*/

#ifndef OS_LLFLOATEREXPORT_H
#define OS_LLFLOATEREXPORT_H

#include "llfloater.h"
#include "llselectmgr.h"
#include "llvoavatar.h"
#include "llavatarappearancedefines.h"
#include "statemachine/aifilepicker.h"
#include "llvoinventorylistener.h" //INV

//<Mesh>
struct MeshDetails
{
	LLUUID id;
	std::string name;
};
//</Mesh>

class LLScrollListCtrl;
class OSExportable : public LLVOInventoryListener //INV
{
	enum EXPORTABLE_TYPE
	{
		EXPORTABLE_OBJECT,
		EXPORTABLE_WEARABLE
	};

public:
	OSExportable(LLViewerObject* object);
	OSExportable(LLViewerObject* object, std::string name, std::map<U32,std::pair<std::string, std::string> >& primNameMap);
	OSExportable(LLVOAvatar* avatar, LLWearableType::EType type, std::map<U32,std::pair<std::string, std::string> >& primNameMap);

	/*virtual*/ void inventoryChanged(LLViewerObject* obj, LLInventoryObject::object_list_t* inv, S32, void*); //INV
	
	LLSD asLLSD();
	
	void requestInventoryFor(LLViewerObject* object); //INV
	void requestInventoriesFor(LLViewerObject* object); //INV

	EXPORTABLE_TYPE mType;
	LLWearableType::EType mWearableType;
	LLViewerObject* mObject;
	LLVOAvatar* mAvatar;
	std::map<U32,std::pair<std::string, std::string> >* mPrimNameMap;
};

class OSFloaterExport : public LLFloater, public LLFloaterSingleton<OSFloaterExport>
{
	friend class LLUISingleton<OSFloaterExport, VisibilityPolicy<LLFloater> >;
public:

	void onOpen();
	virtual BOOL postBuild();

	virtual void draw();
	virtual void refresh();

	void updateSelection();
	void updateAvatarList();
	void updateNamesProgress(bool deselect = false);
	void onClickSelectAll();
	void onClickSelectObjects();
	void onClickSelectWearables();
	//<Mesh>
	void onClickSelectMeshes();
	//</Mesh>
	void onClickMakeCopy();
	void onClickSaveAs();

	void addAvatarStuff(LLVOAvatar* avatarp);
	void receivePrimName(LLViewerObject* object, std::string name, std::string desc);

	static void mirror(const LLUUID& parent_id, const LLUUID& item_id, std::string dir, std::string asset_name);
	static void onClickSaveAs_Callback(OSFloaterExport* floater, AIFilePicker* filepicker);
	static void receiveObjectProperties(LLUUID fullid, std::string name, std::string desc);
	static std::vector<OSFloaterExport*> instances; // for callback-type use

	LLSD getLLSD();
	std::vector<U32> mPrimList;
	LLScrollListCtrl* mExportList;
	std::map<U32, std::pair<std::string, std::string> > mPrimNameMap;
private:
	// INV
	static void assetCallback(LLVFS *vfs,
				   const LLUUID& asset_uuid,
				   LLAssetType::EType type,
				   void* user_data, S32 status, LLExtStat ext_status);

	std::map<LLViewerObject*, bool> avatars;
	LLUUID mObjectID;
	bool mIsAvatar;
	bool mDirty;
	void dirty();

	OSFloaterExport(const LLSD&);
	virtual ~OSFloaterExport(void);

	void addToPrimList(LLViewerObject* object);

	enum LIST_COLUMN_ORDER
	{
		LIST_CHECKED,
		LIST_TYPE,
		LIST_NAME,
		LIST_AVATARID
	};

public:
	void replaceExportableValue(LLUUID id, LLSD data);	//INV
	//void addToExportables(LLUUID id, LLSD data);
	std::map<LLUUID, LLSD> mExportables;
	LLSafeHandle<LLObjectSelection> mObjectSelection;
	//<Mesh>
	static std::map<LLUUID, MeshDetails> mMeshs;
	//</Mesh>
};

#endif //OS_FLOATEREXPORT_H

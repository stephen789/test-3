/** 
* @file llfloaterinspecttexture.h

* $/LicenseInfo$
*/

#ifndef LL_LLFLOATERINSPECTTEXTURE_H
#define LL_LLFLOATERINSPECTTEXTURE_H

#include "llfloater.h"
#include "llscrolllistctrl.h"
#include "lltexturectrl.h"
#include "llviewerobject.h"

class LLObjectSelection;
class LLScrollListCtrl;
class LLUICtrl;

class LLFloaterInspectTexture : public LLFloater, public LLFloaterSingleton<LLFloaterInspectTexture>
{
	friend class LLUISingleton<LLFloaterInspectTexture, VisibilityPolicy<LLFloater> >;
public:

	void onOpen();

	virtual BOOL postBuild();
	LLUUID getSelectedUUID();
	virtual void draw();
	virtual void refresh();
	void updateSelection();
	void onClickCpToInvSelected();
	void onClickCpToInvAll();
	void onSelectTexture();
	void onClickRipTextureAnim();
	void onClickRipParticle();
	void onClickCpUuid();
	void onClickCpAllUuid();
	static void particle_rip(const LLSD& notification, const LLSD& response, LLViewerObject* src_object);
	static void animtext_rip(const LLSD& notification, const LLSD& response, LLViewerObject* src_object);
	LLScrollListCtrl* mTextureList;
	std::string mStatsText;
	S32 mStatsMemoryTotal;
	LLTextureCtrl*    texture_ctrl;
	std::map<LLUUID, bool> unique_textures;

private:
	LLUUID mObjectID;
	bool mIsAvatar;
	bool mDirty;
	void dirty();
	void iterateObjects(LLViewerObject* object, U8 te_count);
	void addToList(const LLUUID& uuid, const LLUUID& uploader, LLViewerObject* obj, S32 height, S32 width, S32 components, const std::string& type_str);
	LLFloaterInspectTexture(const LLSD&);
	virtual ~LLFloaterInspectTexture(void);
	LLSafeHandle<LLObjectSelection> mObjectSelection;
};

#endif //LL_LLFLOATERINSPECTTEXTURE_H

/*
 * 
 * @file os_floaterbuildingprefs.h
 * @author simms
 *
 * ----------------------------------------------------------------------------
 * "The DILLIGAFF license" (Do I Look Like I Give A Flying Fuck):
 * Copyright (c) 2012, Simms, Inc.
 * 
 * If you can throw me some shrapnel for beer and coffee https://flattr.com/donation/give/to/simms
 * ----------------------------------------------------------------------------
 */

#ifndef LL_LLFLOATERBUILDPREFS_H
#define LL_LLFLOATERBUILDPREFS_H

#include "llfloater.h"

class LLCheckBoxCtrl;

class LLFloaterBuildPrefs : public LLFloater, public LLFloaterSingleton<LLFloaterBuildPrefs>
{
public:

	LLFloaterBuildPrefs(const LLSD& seed);
    ~LLFloaterBuildPrefs();

private:
	void apply();
	void cancel();
	void refresh();
	void refreshValues();

	void onBtnOK();
	void onBtnCancel();
	void onBtnApply();

protected:
	void onCommitCheckBox(LLUICtrl* ctrl, const LLSD& value);
	void onCommitComboBox(LLUICtrl* ctrl, const LLSD& value);
	void onCommitTexturePicker(LLUICtrl* ctrl);

private:
	//Build -------------------------------------------------------------------------------
	F32 mAlpha;
	LLColor4 mColor;
	BOOL mFullBright;
	F32 mGlow;
	std::string mItem;
	std::string mMaterial;
	bool mNextCopy;
	bool mNextMod;
	bool mNextTrans;
	bool mScriptNextCopy;
	bool mScriptNextMod;
	bool mScriptNextTrans;
	std::string mShiny;
	bool mTemporary;
	std::string mTexture;
	bool mPhantom;
	bool mPhysical;
	F32 mXsize;
	F32 mYsize;
	F32 mZsize;
	// <os>
	LLCheckBoxCtrl	*mSpoofBuildPrim;
	void onCheckSpoofPrim(const LLSD& value);
	// </os>
};

#endif

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

#ifndef LL_FLOATER_OBJECTFUNCTIONS_H
#define LL_FLOATER_OBJECTFUNCTIONS_H

#include "llfloater.h"

class LLButton;
class LLComboBox;
class LLObjectSelection;

class LLFloaterObjectFunctions : public LLFloater, public LLFloaterSingleton<LLFloaterObjectFunctions>
{
public:
	LLFloaterObjectFunctions(const LLSD& seed);
	~LLFloaterObjectFunctions();
	void onOpen();
	virtual BOOL postBuild();
	LLUUID getSelectedUUID();
	virtual void draw();
	virtual void refresh();
	void updateSelection();

private:
	LLSafeHandle<LLObjectSelection> mObjectSelection;
	void dirty();
	bool mDirty;

	void onClickTakeCopy();
	void onClickTake();
	void onClickDuplicate();
	void onClickDelete();
	void onClickReturn();
	void onClickBlink();
	void onClickLinkObj();
	void onClickUnlinkObj();
	void onClickTextures();
	void onClickExportXml();
	void onClickTouch();
	void onClickSit();
	
	LLButton *mBtnTakeCopy;
	LLButton *mBtnTake;
	LLButton *mBtnDuplicateObj;
	LLButton *mBtnDelete;
	LLButton *mBtnReturn;
	LLButton *mBtnBlink;
	LLButton *mBtnLinkObj;
	LLButton *mBtnUnlinkObj;
	LLButton *mBtnTextures;
	LLButton *mBtnExportXml;
	LLButton *mBtnTouch;
	LLButton *mBtnSit;

};

#endif
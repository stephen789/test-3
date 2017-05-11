/**
 * @file os_floatertoolbox.h
 * @brief Friend-related actions (add, remove, offer teleport, etc)
 *
 * $LicenseInfo:firstyear=2009&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 *
 * Copyright (C) 2014, Danny Simms.
 *
 * $/LicenseInfo$
 */

#ifndef OS_FLOATERTOOLS_H
#define OS_FLOATERTOOLS_H

#include "llfloater.h"
#include "llavatarpropertiesprocessor.h"//online status
#include "llscrolllistctrl.h"//Follow Prims
#include "llviewerobject.h"//Touch Spammer
#include "lleventtimer.h"//Touch Spammer

//online status
enum OnlineStatus
{
	ONLINE_STATUS_NOT = 0,
	ONLINE_STATUS_IS = 1
};

class LLAvatarName;
class LLCheckBoxCtrl;
class LLComboBox;
class LLLineEditor;
class LLTextBox;
class TouchTimer;//Touch Spammer
class FollowPrimList;//Follow Prim

class OSFloaterTools
	: public LLFloater,
	  public LLAvatarPropertiesObserver,//online status
	  public LLEventTimer
{
public:
	OSFloaterTools();
	BOOL postBuild(void);
	void refresh();
	virtual BOOL tick();
	/*virtual*/ void processProperties(void* data, EAvatarProcessorType type);//online status

	void close(bool app_quitting);
	void setTargetAvatar(const LLUUID& target_id);
	void callbackAvatarID(const uuid_vec_t& ids, const std::vector<LLAvatarName> names);
	void onCommitSetText();
	void onClickSet();
	void onClickSendPacket();
	void onClickProfile();
	void onClickPlaySound();
	void onClickAssetOpen();
	void onClickTpGround();
	void onClickTpSafe();
	void onClickUndeform();
	void onClickBlink();
	void onClickDeleteAll();
	void onClickzTake();
	void onClickRezPlat();
	void onClickRez();
	void onClickMegaPrim();
	void onClickDuplicate();
	void onClickAddFollowPrim();//Follow Prims
	void onClickRemoveFollowPrim();//Follow Prims
	void onClickDerez();
	void onClickUpdateChkbox();
	void onCommitDerezCombo(LLUICtrl* ctrl);
	void onCommitFunctionCombo(LLUICtrl* ctrl);
	void onCommitSoundCombo(LLUICtrl* ctrl);
	void setOnlineStatus(OnlineStatus online_status);//online status
	void sendAvatarPropertiesRequest();//online status

private:
	virtual ~OSFloaterTools();

	// static stuff!
public:
	static OSFloaterTools* sInstance;
	static void toggle();
	static BOOL visible();
	static TouchTimer* mTouchTimer;//Touch Spammer
	static void onClickObjPicker(void *userdata);//Touch Spammer
	static void closePickTool(void *userdata);//Touch Spammer
	static void stopTouchSpam(void *userdata);//Touch Spammer
	
	LLUUID getTargetAvatarID(){ return mTargetAvatar; }
	std::string getTargetAvatarName(){ return mTargetAvatarName; }
	S32 getPacketType(){ return packetType; }
	BOOL getSoundList(){ return mSoundList; }
	BOOL getSoundEcho(){ return mSoundEcho; }
	BOOL getSoundLoop(){ return mSoundLoop; }
	BOOL getRezLoop(){ return mRezLoop; }
	BOOL getRdmPos(){ return mRdmPos; }

	LLTextBox* mCounterText;

	enum FOLLOWPRIM_COLUMN_ORDER//Follow Prims
	{
		LIST_FPLAV_NAME,
		LIST_FPL_ICON,
		LIST_FPLOBJECTSID
	};

protected:
	LLLineEditor* mTextEditor;
	LLComboBox* mFunctionComboBox;
	LLComboBox* mSoundComboBox;
	LLComboBox* mDerezComboBox;
	LLCheckBoxCtrl* mSoundChkBox;
	LLCheckBoxCtrl* mEchoChkBox;
	LLCheckBoxCtrl* mSndLoopChkBox;
	LLCheckBoxCtrl* mRezLoopChkBox;
	LLCheckBoxCtrl* mRdmPosChkBox;
	LLButton* mSetTargetBtn;
	LLButton* mSendPacketBtn;
	LLButton* mProfileBtn;
	LLButton* mPlaySoundBtn;
	LLButton* mOpenAssetBtn;
	LLButton* mTpGroundBtn;
	LLButton* mSafeZoneBtn;
	LLButton* mUndeformBtn;
	LLButton* mBlinkBtn;
	LLButton* mDeleteAllBtn;
	LLButton* mzTakeBtn;
	LLButton* mRezPlatBtn;
	LLButton* mRezBtn;
	LLButton* mMegaPrimBtn;
	LLButton* mDuplicateBtn;
	LLButton* mAddFollowPrimBtn;//Follow Prims
	LLButton* mRemoveFollowPrimBtn;//Follow Prims
	LLButton *mBtnDerez;

	LLUUID mTargetAvatar;
	std::string mTargetAvatarName;
	LLScrollListCtrl* mFollowPrimList; //Follow Prims
	std::map<LLUUID, FollowPrimList> mFollowPrims;//Follow Prims
	S32 packetType;
	BOOL mIsFriend;
	BOOL mSoundList;
	BOOL mSoundEcho;
	BOOL mSoundLoop;
	BOOL mRezLoop;
	BOOL mRdmPos;
	S32 DerezType;
};

//Touch Spammer//
class TouchTimer : public LLEventTimer
{
public:
	TouchTimer(LLViewerObject* object);
	virtual ~TouchTimer();
	//function to be called at the supplied frequency
	virtual BOOL tick();
	std::string getTarget();
private:
	LLViewerObject* myObject;
};

//Follow Prims//
class FollowPrimList
{
public:
	FollowPrimList(const LLUUID& AvatarID = LLUUID::null, const std::string& AvatarName = "", const LLUUID& ObjectID = LLUUID::null, const U32& ObjectLocalID = (U32)0);
	std::string getAvatarName();
	void setAvatarName(std::string name);
	LLUUID getAvatarID();
	void setAvatarID(LLUUID AvatarID);
	LLUUID getObjectID();
	void setObjectID(LLUUID ObjectID);
	U32 getObjectLocalID();
	void setObjectLocalID(U32 ObjectLocalID);
private:
	friend class OSFloaterTools;
	LLUUID mAvatarsID;
	std::string mAvatarsName;
	LLUUID mObjectsID;
	U32 mObjectsLocalID;
};

#endif // OS_FLOATERTOOLS_H
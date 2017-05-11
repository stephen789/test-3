/**
* @file os_floatertoolbox.cpp
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

/* basic headers */
#include "llviewerprecompiledheaders.h"
#include "os_floatertoolbox.h"

/* llui headers */
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "lllineeditor.h"
#include "lltextbox.h"
#include "llscrolllistitem.h"
#include "lluictrlfactory.h"

/* misc headers */

#include "llmath.h"

#include "llworld.h"//TEST
#include "llagent.h"
#include "llavatarnamecache.h"
#include "llfloateravatarpicker.h"
#include "lleventtimer.h"
#include "llviewerregion.h"
#include "llviewerstats.h"
#include "llinstantmessage.h"
#include "llviewermessage.h"
#include "lltransactionflags.h"
#include "llcallingcard.h"
#include "llavataractions.h"
#include "llavatarconstants.h"
#include "llfloateravatarinfo.h"
#include "lltoolmgr.h"
#include "lltoolobjpicker.h"
#include "llviewerobjectlist.h"
#include "llviewerobject.h"
#include "lltooldraganddrop.h"
#include "llvolumemessage.h"
#include "llselectmgr.h"
#include "os_floaterkeytool.h"
#include "llfloatertools.h" //gfloatertools
#include "chatbar_as_cmdline.h"
#include "llviewerwindow.h"
#include "llwindow.h"

TouchTimer* OSFloaterTools::mTouchTimer = NULL;//Touch Spammer

/*=======================================*/
/*  PacketTimer : packet timer class     */
/*=======================================*/
BOOL sendTypingStop = FALSE;
class PacketTimer : public LLEventTimer
{
public:
	PacketTimer();
	virtual ~PacketTimer();
	//function to be called at the supplied frequency
	virtual BOOL tick();
	U32 counter;
private:
	std::string textBoxInput;
	U32 howMany;
	LLUUID TargetAvatarKey;
	std::string TargetAvatarName;
	EInstantMessage mIMType;
	bool loop_active;
	bool rezloop_active;
	bool sndloop_active;
};

PacketTimer::PacketTimer() : LLEventTimer((F32)(0.3)), counter(0)
{
	howMany = gSavedSettings.getU32("OSToolNumPackets");
	textBoxInput = gSavedSettings.getString("OSToolTextEditor");
	TargetAvatarKey = OSFloaterTools::sInstance->getTargetAvatarID();
	TargetAvatarName = OSFloaterTools::sInstance->getTargetAvatarName();
	mIMType = (EInstantMessage)OSFloaterTools::sInstance->getPacketType();
	loop_active = OSFloaterTools::sInstance->childGetValue("loop_chk").asBoolean();
	rezloop_active = OSFloaterTools::sInstance->getRezLoop();
	sndloop_active = OSFloaterTools::sInstance->getSoundLoop();

};
PacketTimer::~PacketTimer()
{
}


BOOL PacketTimer::tick()
{
	//Non Packet Shit
	if (rezloop_active || sndloop_active)
	{
		if (OSFloaterTools::sInstance->getRezLoop())
		{
			howMany = (U32)OSFloaterTools::sInstance->childGetValue("Amount").asInteger();
			OSFloaterTools::sInstance->onClickRez();
		}
		if (sndloop_active)
		{
			howMany = (U32)OSFloaterTools::sInstance->childGetValue("sound_amount").asInteger();
			OSFloaterTools::sInstance->onClickPlaySound();
		}
	}

	if (sendTypingStop)
	{
		if (counter < howMany || loop_active)
		{
			if (rezloop_active)
			{
				OSFloaterTools::sInstance->onClickRez();
			}
			if (OSFloaterTools::sInstance->getSoundLoop())
			{
				OSFloaterTools::sInstance->onClickPlaySound();
			}

			if (mIMType == 69)
			{
				mIMType = (EInstantMessage)ll_rand(50); //randomize packets
			}
			else if (mIMType == IM_LURE_USER)
			{ 
				gMessageSystem->newMessageFast(_PREHASH_StartLure);
				gMessageSystem->nextBlockFast(_PREHASH_AgentData);
				gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
				gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
				gMessageSystem->nextBlockFast(_PREHASH_Info);
				gMessageSystem->addU8Fast(_PREHASH_LureType, (U8)0);
				gMessageSystem->addStringFast(_PREHASH_Message, textBoxInput);
				gMessageSystem->nextBlockFast(_PREHASH_TargetData);
				gMessageSystem->addUUIDFast(_PREHASH_TargetID, TargetAvatarKey);
				gAgent.sendReliableMessage();
			}
			else if (mIMType == IM_FRIENDSHIP_OFFERED)
			{
				const LLUUID fid = gInventory.findCategoryUUIDForType(LLFolderType::FT_CALLINGCARD);
				send_improved_im(TargetAvatarKey, TargetAvatarName, textBoxInput, IM_ONLINE, mIMType, fid);
			}
			else if (mIMType == 45)
			{
				LLUUID transaction_id;
				transaction_id.generate();
				gMessageSystem->newMessage("OfferCallingCard");
				gMessageSystem->nextBlockFast(_PREHASH_AgentData);
				gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
				gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
				gMessageSystem->nextBlockFast(_PREHASH_AgentBlock);
				gMessageSystem->addUUIDFast(_PREHASH_DestID, TargetAvatarKey);
				gMessageSystem->addUUIDFast(_PREHASH_TransactionID, transaction_id);
				gAgent.sendMessage();
			}
			else if (mIMType == 46)
			{
				LLMessageSystem* msg = gMessageSystem;
				std::string request = "simulatormessage";
				LLUUID invoice;
				invoice.generate();

				LL_INFOS()<< "Sending estate request '" << request << "'" << LL_ENDL;
				msg->newMessage("EstateOwnerMessage");
				msg->nextBlockFast(_PREHASH_AgentData);
				msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
				msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
				msg->addUUIDFast(_PREHASH_TransactionID, LLUUID::null); //not used
				msg->nextBlock("MethodData");
				msg->addString("Method", request);
				msg->addUUID("Invoice", invoice);

				msg->nextBlock("ParamList");
				msg->addString("Parameter", "-1");
				msg->addString("Parameter", "-1");
				msg->addString("Parameter", TargetAvatarKey.asString().c_str());
				msg->addString("Parameter", TargetAvatarName);
				msg->addString("Parameter", textBoxInput);
			}
			else if (mIMType == 47)
			{
				LLMessageSystem* msg = gMessageSystem;
				msg->newMessageFast(_PREHASH_MoneyTransferRequest);
				msg->nextBlockFast(_PREHASH_AgentData);
				msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
				msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
				msg->nextBlockFast(_PREHASH_MoneyData);
				msg->addUUIDFast(_PREHASH_SourceID, gAgent.getID());
				msg->addUUIDFast(_PREHASH_DestID, TargetAvatarKey);
				msg->addU8Fast(_PREHASH_Flags, pack_transaction_flags(FALSE, 0));// is_group));
				msg->addS32Fast(_PREHASH_Amount, atoi(textBoxInput.c_str()));
				msg->addU8Fast(_PREHASH_AggregatePermNextOwner, (U8)LLAggregatePermissions::AP_EMPTY);
				msg->addU8Fast(_PREHASH_AggregatePermInventory, (U8)LLAggregatePermissions::AP_EMPTY);
				msg->addS32Fast(_PREHASH_TransactionType, TRANS_GIFT);
				msg->addStringFast(_PREHASH_Description, "");
				gAgent.sendReliableMessage();
			}
			else
			{
				std::string name;
				gAgent.buildFullname(name);
				pack_instant_message(
				gMessageSystem,
				gAgent.getID(),
				FALSE,
				gAgent.getSessionID(),
				TargetAvatarKey,
				name,
				textBoxInput,
				IM_ONLINE,
				mIMType,
				LLUUID::null);
				gAgent.sendReliableMessage();
			}
			counter++;
			OSFloaterTools::sInstance->mCounterText->setText(llformat("Packets Sent: %d", counter));
			return FALSE;
		}
		else
		{
			sendTypingStop = FALSE;
			return TRUE;
		}
	}
	else
	return TRUE;
}

/*=======================================*/
/*  FollowPrimList : follow prims class	 */
/*=======================================*/

FollowPrimList::FollowPrimList(const LLUUID& AvatarID, const std::string& AvatarName, const LLUUID& ObjectID, const U32& ObjLocalID) :
mAvatarsID(AvatarID), mAvatarsName(AvatarName), mObjectsID(ObjectID), mObjectsLocalID(ObjLocalID)
{
}

void FollowPrimList::setAvatarName(std::string name)
{
	if (name.empty() || (name.compare(" ") == 0))
	{
		//llwarns << "Trying to set empty name" << LL_ENDL;
	}
	mAvatarsName = name;
}

std::string FollowPrimList::getAvatarName()
{
	return mAvatarsName;
}

LLUUID FollowPrimList::getAvatarID()
{
	return mAvatarsID;
}

void FollowPrimList::setObjectID(LLUUID id)
{
	if (id.isNull())
	{
		llwarns << "Trying to set null id1" << LL_ENDL;
	}
	mObjectsID = id;
}

LLUUID FollowPrimList::getObjectID()
{
	return mObjectsID;
}

void FollowPrimList::setObjectLocalID(U32 id)
{
	mObjectsLocalID = id;
}

U32 FollowPrimList::getObjectLocalID()
{
	return mObjectsLocalID;
}

/*=======================================*/
/*  OSFloaterTools : main floater class	 */
/*=======================================*/
OSFloaterTools* OSFloaterTools::sInstance;
OSFloaterTools::OSFloaterTools()
	: LLFloater(), 
	LLEventTimer(0.025f),
	mTargetAvatar()
{
	// xui creation:
	LLUICtrlFactory::getInstance()->buildFloater(this, "os_floater_tools.xml");

	// creating Instance
	OSFloaterTools::sInstance = this;
}

OSFloaterTools::~OSFloaterTools()
{
	OSFloaterTools::sInstance = NULL;
}

// static
void OSFloaterTools::toggle()
{
	if (OSFloaterTools::sInstance) OSFloaterTools::sInstance->close(FALSE);
	else OSFloaterTools::sInstance = new OSFloaterTools();
}

BOOL OSFloaterTools::visible()
{
	if (OSFloaterTools::sInstance)
		return TRUE;
	return FALSE;
}

void OSFloaterTools::close(bool app_quitting)
{
	if (mTargetAvatar.notNull()) LLAvatarPropertiesProcessor::getInstance()->removeObserver(mTargetAvatar, this);//online status
	LLFloater::close(app_quitting);
}

BOOL OSFloaterTools::postBuild(void)
{
	// Bools
	mSoundList = FALSE;
	mSoundEcho = FALSE;
	mSoundLoop = FALSE;
	mRezLoop = FALSE;
	mRdmPos = FALSE;

	// setting element/xui children:
	mCounterText = getChild<LLTextBox>("counter");
	mTextEditor = getChild<LLLineEditor>("text_editor");
	mFunctionComboBox = getChild<LLComboBox>("function_combobox");
	mSoundComboBox = getChild<LLComboBox>("sound_combobox");
	mDerezComboBox = getChild<LLComboBox>("derez_combobox");
	mSoundChkBox = getChild<LLCheckBoxCtrl>("sound_checkbox");
	mSndLoopChkBox = getChild<LLCheckBoxCtrl>("snd_loop");
	mEchoChkBox = getChild<LLCheckBoxCtrl>("echo_checkbox");
	mRezLoopChkBox = getChild<LLCheckBoxCtrl>("rez_loop");
	mRdmPosChkBox = getChild<LLCheckBoxCtrl>("rdm_pos");
	mSetTargetBtn = getChild<LLButton>("set_target_btn");
	mSendPacketBtn = getChild<LLButton>("send_packet_btn");
	mProfileBtn = getChild<LLButton>("profile_btn");
	mPlaySoundBtn = getChild<LLButton>("play_sound_button");
	mOpenAssetBtn = getChild<LLButton>("open_asset_button");
	mTpGroundBtn = getChild<LLButton>("tp_ground_button");
	mSafeZoneBtn = getChild<LLButton>("tp_safe_button");
	mUndeformBtn = getChild<LLButton>("undeform_button");
	mBlinkBtn = getChild<LLButton>("blink_button");
	mDeleteAllBtn = getChild<LLButton>("delete_all_button");
	mzTakeBtn = getChild<LLButton>("ztake_button");
	mRezPlatBtn = getChild<LLButton>("rezplat_button");
	mRezBtn = getChild<LLButton>("rez_button");
	mMegaPrimBtn = getChild<LLButton>("megaprim_button");
	mDuplicateBtn = getChild<LLButton>("duplicate_button");
	mAddFollowPrimBtn = getChild<LLButton>("addFollow_button");
	mRemoveFollowPrimBtn = getChild<LLButton>("removeFollow_button");
	mBtnDerez = getChild<LLButton>("derez_obj");

	//touch spam
	std::string status = mTouchTimer ? mTouchTimer->getTarget() : "No Target";
	if (strcmp(childGetText("object_name_label").c_str(), status.c_str()) != 0)
		childSetText("object_name_label", status);

	//online status
	childSetVisible("online_yes", FALSE);
	childSetColor("online_yes", LLColor4::green);
	childSetValue("online_yes", "Currently Online");
	//end

	// button callbacks:
	mSetTargetBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickSet, this));
	mSendPacketBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickSendPacket, this));
	mProfileBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickProfile, this));
	mPlaySoundBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickPlaySound, this));
	mOpenAssetBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickAssetOpen, this));
	mTpGroundBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickTpGround, this));
	mSafeZoneBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickTpSafe, this));
	mUndeformBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickUndeform, this));
	mBlinkBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickBlink, this));
	mDeleteAllBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickDeleteAll, this));
	mzTakeBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickzTake, this));
	mRezPlatBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickRezPlat, this));
	mRezBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickRez, this));
	mMegaPrimBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickMegaPrim, this));
	mDuplicateBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickDuplicate, this));
	mAddFollowPrimBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickAddFollowPrim, this));
	mRemoveFollowPrimBtn->setClickedCallback(boost::bind(&OSFloaterTools::onClickRemoveFollowPrim, this));
	mBtnDerez->setClickedCallback(boost::bind(&OSFloaterTools::onClickDerez, this));

	// combo callback
	packetType = IM_NOTHING_SPECIAL;
	mFunctionComboBox->add("Instant Message", LLSD(0));
	/*
	mFunctionComboBox->add("IM_MESSAGEBOX", std::string("1"));
	mFunctionComboBox->add("IM_MESSAGEBOX_COUNTDOWN", std::string("2"));
	mFunctionComboBox->add("IM_GROUP_INVITATION", std::string("3"));
	mFunctionComboBox->add("IM_INVENTORY_OFFERED", std::string("4"));
	mFunctionComboBox->add("IM_INVENTORY_ACCEPTED", std::string("5"));
	mFunctionComboBox->add("IM_INVENTORY_DECLINED", std::string("6"));
	mFunctionComboBox->add("IM_GROUP_VOTE", std::string("7"));
	mFunctionComboBox->add("IM_GROUP_MESSAGE_DEPRECATED", std::string("8"));
	mFunctionComboBox->add("IM_TASK_INVENTORY_OFFERED", std::string("9"));
	mFunctionComboBox->add("IM_TASK_INVENTORY_ACCEPTED", std::string("10"));
	mFunctionComboBox->add("IM_TASK_INVENTORY_DECLINED", std::string("11"));
	mFunctionComboBox->add("IM_NEW_USER_DEFAULT", std::string("12"));
	mFunctionComboBox->add("IM_SESSION_INVITE", std::string("13"));
	mFunctionComboBox->add("IM_SESSION_P2P_INVITE", std::string("14"));
	mFunctionComboBox->add("IM_SESSION_GROUP_START", std::string("15"));
	mFunctionComboBox->add("IM_SESSION_CONFERENCE_START", std::string("16"));
	mFunctionComboBox->add("IM_SESSION_SEND", std::string("17"));
	mFunctionComboBox->add("IM_SESSION_LEAVE", std::string("18"));
	mFunctionComboBox->add("IM_FROM_TASK", std::string("19"));
	*/
	mFunctionComboBox->add("Busy Response", std::string("20"));
	
	//mFunctionComboBox->add("IM_CONSOLE_AND_CHAT_HISTORY", std::string("21"));
	mFunctionComboBox->add("Offer Teleport", std::string("22"));
	//mFunctionComboBox->add("IM_LURE_ACCEPTED", std::string("23"));
	//mFunctionComboBox->add("IM_LURE_DECLINED", std::string("24"));
	/*
	mFunctionComboBox->add("IM_GODLIKE_LURE_USER", std::string("25"));
	*/
	mFunctionComboBox->add("Request Teleport", std::string("26"));
	/*
	mFunctionComboBox->add("IM_GROUP_ELECTION_DEPRECATED", std::string("27"));
	mFunctionComboBox->add("IM_GOTO_URL", std::string("28"));
	mFunctionComboBox->add("IM_FROM_TASK_AS_ALERT", std::string("31"));
	mFunctionComboBox->add("IM_GROUP_NOTICE", std::string("32"));
	mFunctionComboBox->add("IM_GROUP_NOTICE_INVENTORY_ACCEPTED", std::string("33"));
	mFunctionComboBox->add("IM_GROUP_NOTICE_INVENTORY_DECLINED", std::string("34"));
	mFunctionComboBox->add("IM_GROUP_INVITATION_ACCEPT", std::string("35"));
	mFunctionComboBox->add("IM_GROUP_INVITATION_DECLINE", std::string("36"));
	mFunctionComboBox->add("IM_GROUP_NOTICE_REQUESTED", std::string("37"));
	*/
	mFunctionComboBox->add("Offer Friendship", std::string("38"));
//	mFunctionComboBox->add("IM_FRIENDSHIP_ACCEPTED", std::string("39"));
//	mFunctionComboBox->add("IM_FRIENDSHIP_DECLINED_DEPRECATED", std::string("40"));
	mFunctionComboBox->add("TYPING::START", std::string("41"));
	mFunctionComboBox->add("TYPING::STOP", std::string("42"));

	//mFunctionComboBox->add("IM_SESSION_IRC_START", std::string("43"));
	//mFunctionComboBox->add("IM_PRIVATE_IRC", std::string("44"));
	//Custom Over 44
	mFunctionComboBox->add("Offer Calling Card", std::string("45"));
	mFunctionComboBox->add("Simulator Message", std::string("46"));
	mFunctionComboBox->add("Give Money", std::string("47"));
	mFunctionComboBox->add("Randomized Packet", std::string("69"));
	mFunctionComboBox->setCommitCallback(boost::bind(&OSFloaterTools::onCommitFunctionCombo, this, _1));

	mSoundComboBox->add("SL IM", std::string("ec3fe6e4-38e1-015b-d987-284393bf7a55"));
	mSoundComboBox->add("SL Teleportation", std::string("d7a9a565-a013-2a69-797d-5332baa1a947"));
	mSoundComboBox->add("SL Typing", std::string("5e191c7b-8996-9ced-a177-b2ac32bfea06"));
	mSoundComboBox->add("SL Snapshot", std::string("3d09f582-3851-c0e0-f5ba-277ac5c73fb4"));
	mSoundComboBox->add("SL Cash register", std::string("104974e3-dfda-428b-99ee-b0d4e748d3a3"));
	mSoundComboBox->add("SL Big Cash Payout", std::string("77a018af-098e-c037-51a6-178f05877c6f"));
	mSoundComboBox->add("Strings?", std::string("00a31197-3c8a-2d76-1ea0-09372a592ad7"));
	mSoundComboBox->add("Birds", std::string("84352864-b7c0-f957-7eac-0b2c6788df49"));
	mSoundComboBox->add("More birds", std::string("6ddee70d-e902-b926-3188-c73ebde2bb11"));
	mSoundComboBox->add("Rock the Casbah pt1", std::string("3eca3ec5-88b8-6780-9bb1-0728fab98831"));
	mSoundComboBox->add("Rock the Casbah pt2", std::string("5e7ab2d0-b80c-4fe8-e10e-21c86e6fa286"));
	mSoundComboBox->add("ADDITIONAL PYLONS", std::string("7f4d5177-88da-ed23-de6c-f39711a9befb"));
	mSoundComboBox->add("Air raid siren", std::string("a8f97b16-7c1f-9409-d738-ac1c15efa672"));
	mSoundComboBox->add("AMERICA, FUCK YEAH", std::string("83b2512a-8966-2d08-12b7-4899359b7788"));
	mSoundComboBox->add("Amerika(Rammstein)", std::string("9b8af295-576f-2069-180d-5d39269b8bea"));
	mSoundComboBox->add("WE ARE ANONYMOUS", std::string("40c17dbd-1240-d27e-da8f-69b85005034e"));
	mSoundComboBox->add("Bananaphone", std::string("da73f914-c96a-7f5d-f4b4-6e4c711cba3d"));
	mSoundComboBox->add("Bananaphone backwards", std::string("38f893aa-0566-b304-1a1f-93e06218482a"));
	mSoundComboBox->add("BAWWWWWWWWWWWWWW", std::string("b4acf252-2aeb-3b62-c18d-056de4e9962d"));
	mSoundComboBox->add("Bigben", std::string("c3b45204-3e81-17f1-de7e-d628ae4e25ad"));
	mSoundComboBox->add("Bigben, Inverted and Messed Up", std::string("9498c94c-eae7-5c17-a9a6-3a4ce3ab213c"));
	mSoundComboBox->add("Bill Clinton : It depends on what the meaning of the word 'is' is.", std::string("5a87051c-55dc-e500-3dd1-77df530189de"));
	mSoundComboBox->add("Breaking Glass and Trashing Stuff", std::string("4b10dc54-5f5d-e539-99f6-94044116144e"));
	mSoundComboBox->add("Burn Bobonga(Brian Peppers)", std::string("22a345bb-e5d4-2390-3a08-1eede62822de"));
	mSoundComboBox->add(" But I poop from there", std::string("5d411f4d-cbcb-99cd-cde7-5d919b4d5140"));
	mSoundComboBox->add("Captain Planet", std::string("7b8bc0cd-b6b4-68a2-b66d-97e1b3f40e7e"));
	mSoundComboBox->add("CARAMELLDANSEN", std::string("604e1b90-93f1-2bd5-f328-362629309cfe"));
	mSoundComboBox->add("Cartman says GET YOUR BITCH ASS B2 / KITCHEN", std::string("5eb749ac-aa4c-adf9-5106-025cfdc23c8f"));
	mSoundComboBox->add("Chocolate Rain", std::string("4493d86e-ca3b-7f10-1350-83e87fc49dce"));
	mSoundComboBox->add("Choppaman", std::string("6364fda6-a8d9-ac18-eabb-8781da415459"));
	mSoundComboBox->add("Cosby BAH BAH BAH", std::string("70642e15-32b4-62d4-b096-a7875006f446"));
	mSoundComboBox->add("Cosby Show Theme", std::string("9f172398-6fe3-78a9-ae79-b3a8218ad242"));
	mSoundComboBox->add("Cosby WHAT DO YOU LIKE TO PLAY ?", std::string("2a21f4af-d220-88fe-b4e3-f1b6713b2b3b"));
	mSoundComboBox->add("Cosby Cube Original, a combination of the three", std::string("9814cd22-9032-72b1-5eaa-3c18e4b9cd40"));
	mSoundComboBox->add("Chris Hansen RAPE loop", std::string("646617a0-d4b2-13c2-1e41-9ee3eccb9af0"));
	mSoundComboBox->add("Chris Hansen LAUGH OUT LOUD loop", std::string("60deff63-7727-e4b0-d373-cf180ae69a1f"));
	mSoundComboBox->add("Dead niggers", std::string("47135b85-d747-b611-e59c-d0997c5eb9a1"));
	mSoundComboBox->add("Dead niggers (much louder)", std::string("e4829af4-6c9f-f2c2-f6a5-2b1195bcfcf7"));
	mSoundComboBox->add("Dead niggers(More common clip)", std::string("f3695500-0c50-beae-dd82-10521970bf17"));
	mSoundComboBox->add("Dexter's a cookie!", std::string("d7db28cb-e450-b45f-1fb5-4ffa682ac417"));
	mSoundComboBox->add("Dexter's Secret", std::string("519b52a2-7947-1426-3e3a-3ec93cd37ed2"));
	mSoundComboBox->add("Die motherfucker", std::string("0ed44a16-3383-4377-96b1-9a85cad9aa42"));
	mSoundComboBox->add("Do the Mario", std::string("95fd2e68-e6cd-f6e8-d7e9-564d9eccc22e"));
	mSoundComboBox->add("DO THE MARIO(louder)", std::string("99da2887-cfac-a370-569d-83b3edc3a530"));
	mSoundComboBox->add("Doug", std::string("269fbd53-b651-55f8-24f5-34988eecc2af"));
	mSoundComboBox->add("Duke Nukem : It's time to kick ass...", std::string("f4f2d8f9-3dc1-44e0-3c4a-2147ad8bb1e3"));
	mSoundComboBox->add("Duke Nukem : Blow it out yer ass", std::string("c683cbe8-4257-86fc-9638-613102552ca7"));
	mSoundComboBox->add("Explosion", std::string("883c6ec9-074c-4404-b7c3-e02e1462a36e"));
	mSoundComboBox->add("Feuer Frei(Rammstein)", std::string("27d53e50-8e31-8722-6349-6c75577e9b73"));
	mSoundComboBox->add("Fear of God", std::string("d517b1e4-8aea-ee1b-c3b8-46d0d1786527"));
	mSoundComboBox->add("FUCK HIS HEAD", std::string("16659f75-75eb-7187-8fde-d74cef59c55a"));
	mSoundComboBox->add("F**K SPAGHETTI", std::string("c3b2fc1b-144b-79d2-87b5-4cb3a66a5e4e"));
	mSoundComboBox->add("FUCK YEAH, SEAKING", std::string("f9b562b9-6747-3f6a-2ad8-c8d4bf09b582"));
	mSoundComboBox->add("George W.Bush: Fool me once, shame on you.You fool me -- you can't get fooled again.", std::string("d27afb10-133f-494d-4560-3623566c4fd5"));
	mSoundComboBox->add("Gigantor", std::string("070f74dd-d753-19dd-197c-714b8e63e386"));
	mSoundComboBox->add("Glenn Beck wants you to get off his phone.", std::string("f2013897-1fc9-f00d-4c6a-2f40008435c4"));
	mSoundComboBox->add("HOMO, FAG, QUEER, FAGGOOOOT", std::string("505a2b13-029b-41c5-0ae4-7c35920fedc3"));
	mSoundComboBox->add("HRRRRAGLAAAAAAAAA COUNT DRACULAAAAAA", std::string("f783c057-5993-ebbc-a486-99ae5d2949af"));
	mSoundComboBox->add("I will always be borg", std::string("338fe6ef-8de4-1173-6f64-3b0bd55e2d6d"));
	mSoundComboBox->add("I got the power(80's song)", std::string("65d296bd-3d14-45b0-a838-779f2c2376d4"));
	mSoundComboBox->add("I guarantee it", std::string("a7a9140a-ffb0-9a51-c602-00b5aa884784"));
	mSoundComboBox->add("IMMA FIRIN MAH LAZER", std::string("de81d6f5-9236-1f7f-cc58-c2d7f90e0852"));
	mSoundComboBox->add("It's a trap!", std::string("7353c06a-393a-89fc-74e7-ccc08e9b46d7"));
	mSoundComboBox->add("It's over 9000!", std::string("b24847f4-d828-1854-3467-0d4d5433fd2d"));
	mSoundComboBox->add("Jew Music", std::string("8a756921-fa26-9dd2-429e-0b0bdc05a55c"));
	mSoundComboBox->add("Last Measure", std::string("d7c2452f-738d-bed3-5b27-247bd9f811be"));
	mSoundComboBox->add("lol, internet(ytmnd)", std::string("c3895cf3-e635-0031-da00-aa74e2b9dac5"));
	mSoundComboBox->add("Loud shit(like a jet engine)", std::string("cfae2e69-0d87-446c-fa47-12a6ee9a4335"));
	mSoundComboBox->add("MADNESS loop", std::string("38fe29a4-091d-c6a0-3fa5-18ade14e2a82"));
	mSoundComboBox->add("Mama Luigi", std::string("7c3783e2-d4ca-535b-79f6-20c990f6e420"));
	mSoundComboBox->add("Mario Power Up Music", std::string("e5e45765-531c-e2c7-0cf5-76a8cc974bdf"));
	mSoundComboBox->add("Mohammed Hassan intro", std::string("4bcba26e-d72b-643d-fdc3-4fd086737a0d"));
	mSoundComboBox->add("Mudkip loop", std::string("091d6f7d-e42d-6a0e-a001-a21e167bee27"));
	mSoundComboBox->add("NEDM", std::string("e61efc1b-1d4b-97ea-1b14-9360aeac6cdd"));
	mSoundComboBox->add("Oprah Over 9000 Penises", std::string("a0f271c3-4a3f-5ac3-996f-ef8383604254"));
	mSoundComboBox->add("Peach scream", std::string("96be9d2e-7634-881a-da0a-eec7068b5462"));
	mSoundComboBox->add("Picard", std::string("4d2d26f7-9c05-e25e-4334-19be9dd342db"));
	mSoundComboBox->add("Raining Men", std::string("38c34d61-ca32-a828-028a-70a9558a43e9"));
	mSoundComboBox->add("Rapeface RAPE RAPE RAPE Saria's Song", std::string("536b5ec9-f99b-a0f3-7a1c-0a6fc6e778d5"));
	mSoundComboBox->add("RAPESCREAM", std::string("ec0f62f2-d564-28e5-4a4f-076998ad0dd9"));
	mSoundComboBox->add("ROW ROW, FIGHT THE POWER(GL Rap)", std::string("acf118ae-6bfe-9dd7-2435-40a1c365987d"));
	mSoundComboBox->add("Scatman John - Scatman chorus", std::string("da7fd53c-7f99-f5f2-d798-a7fcd2406dda"));
	mSoundComboBox->add("Sieg Heil Loop", std::string("a21c22cf-8722-5312-35dd-834ac948300a"));
	mSoundComboBox->add("Souljaboy", std::string("bd77c568-9a6b-c039-9a3c-1e02c391e187"));
	mSoundComboBox->add("SPAGHETTTTTTTTTTTI", std::string("0c2b918a-941c-ab8f-c7be-77c5c748aa5c"));
	mSoundComboBox->add("SPAGHETTTTTTTTTTTI(Really really loud)", std::string("03acbd80-f62a-d383-54b9-722bc5eb3d9f"));
	mSoundComboBox->add("SPY'S SAPPEN MAH SENTRY", std::string("de7182f4-c564-0b2b-203c-95de920d8bdc"));
	mSoundComboBox->add("Square Wave", std::string("a82888c1-f642-4e44-734d-7b89e4db0578"));
	mSoundComboBox->add("Steppenwolf - Magic Carpet Ride(60 seconds long)", std::string("d65e1e9c-72d9-f334-c0b6-60312718c443"));
	mSoundComboBox->add("Steven Richards : he's a nigger", std::string("9e109a46-98ef-d76a-f193-ac1595c17728"));
	mSoundComboBox->add("S.O.A.D.pizza pie", std::string("5a013991-4b78-1386-4c3e-9fc5ab49c854"));
	mSoundComboBox->add("STOP BREAKING THE LAW ASSHOLE", std::string("9302611a-c44b-3741-ac85-705cb6958523"));
	mSoundComboBox->add("toring dead niggers ain't my fucking business!", std::string("706719a9-1e0a-65cd-4737-acc259661242"));
	mSoundComboBox->add("sucemonpenis", std::string("c91468d2-1251-774d-ecf4-ec0bb0dddce9"));
	mSoundComboBox->add("Super Waha Supergirl Loop", std::string("035a2ee9-c2d5-8030-8987-bc76fc6716c6"));
	mSoundComboBox->add("That's the best thing I ever saw!", std::string("8a1d214b-9b76-90f5-a57e-ef66dce47505"));
	mSoundComboBox->add("THIS IS SPARTA", std::string("499f7c6d-81f7-7ca9-1632-6b573f720e57"));
	mSoundComboBox->add("Tom Green barrel roll loop", std::string("56296d32-38ad-5dd6-bf31-228f107cf6fc"));
	mSoundComboBox->add("T rex roar", std::string("52bd8903-f3eb-dc52-2ba0-c9271ab16ca6"));
	mSoundComboBox->add("Truly epic lulz", std::string("8921eb19-19fc-8fcc-70db-4ad4b2319ba2"));
	mSoundComboBox->add("WAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAGH!", std::string("e4f13c64-0dbe-4c72-8759-6c3f6da22004"));
	mSoundComboBox->add("Wakka wakka wakka", std::string("1cb2cb0e-df53-9837-0a37-9c6ead40b6ef"));
	mSoundComboBox->add("We are the borg.You will be Assimilated.Resistance is futile.", std::string("d736ddb3-6374-bb46-d173-da88c300c1f3"));
	mSoundComboBox->add("What is love", std::string("15eacc40-cc16-4d24-e4ff-ac78c3eae9c2"));
	mSoundComboBox->add("WHERE IN THE WORLD IS PATRIOTIC NIGRAS ?", std::string("2ed4004f-2150-d1eb-d937-35ec32d8495a"));
	mSoundComboBox->add("WHERE THE HOOD AT!?", std::string("20eefa4b-958c-7b15-7484-20bdbe163181"));
	mSoundComboBox->add("White noise", std::string("25d02a0d-f4f5-986f-233b-a7e4caafef56"));
	mSoundComboBox->add("Whoa those guys are fags!", std::string("a0260eb3-d6e9-df66-4bf4-2c3095cda6b8"));
	mSoundComboBox->add("Wonderful time", std::string("480495eb-bb83-4f96-c912-afad67b559c9"));
	mSoundComboBox->add("WRRRRYYYYYYY(New)", std::string("81dbfff8-4ec2-4c59-122a-1fb2a5a0c7e6"));
	mSoundComboBox->add("YOU ARE AN IDIOT!HAHAHA", std::string("03a51ebc-fbd7-a905-e8b0-d178b4ed0f28"));
	mSoundComboBox->add("ANNIHILATION - WAAAAAAAAAAAH", std::string("d20787ec-845b-8f83-b71c-84149ecb1a94"));
	mSoundComboBox->setCommitCallback(boost::bind(&OSFloaterTools::onCommitSoundCombo, this, _1));

	mDerezComboBox->add("Derez Attachment", LLSD(DRD_ATTACHMENT));
	mDerezComboBox->add("Attachment Exists", LLSD(DRD_ATTACHMENT_EXISTS));
	mDerezComboBox->add("Attachment Agent Inventory", LLSD(DRD_ATTACHMENT_TO_INV));
	mDerezComboBox->add("Agent Inventory", LLSD(DRD_SAVE_INTO_AGENT_INVENTORY));
	mDerezComboBox->add("Agent Inventory Copy", LLSD(DRD_ACQUIRE_TO_AGENT_INVENTORY));
	mDerezComboBox->add("Agent Inventory Take", LLSD(DRD_TAKE_INTO_AGENT_INVENTORY));
	mDerezComboBox->add("Object Inventory", LLSD(DRD_SAVE_INTO_TASK_INVENTORY));
	mDerezComboBox->add("Godlike Agent Inventory", LLSD(DRD_FORCE_TO_GOD_INVENTORY));
	mDerezComboBox->add("Move to Trash", LLSD(DRD_TRASH));
	mDerezComboBox->add("Return To Owner", LLSD(DRD_RETURN_TO_OWNER));
	mDerezComboBox->add("Return To LastOwner", LLSD(DRD_RETURN_TO_LAST_OWNER));
	mDerezComboBox->setCommitCallback(boost::bind(&OSFloaterTools::onCommitDerezCombo, this, _1));

	// line editor callbacks
	mTextEditor->setRevertOnEsc(FALSE);
	mTextEditor->setCommitOnFocusLost(TRUE);
	mTextEditor->setCommitCallback(boost::bind(&OSFloaterTools::onCommitSetText, this));

	// checkbox callbacks
	mSoundChkBox->setCommitCallback(boost::bind(&OSFloaterTools::onClickUpdateChkbox, this));
	mEchoChkBox->setCommitCallback(boost::bind(&OSFloaterTools::onClickUpdateChkbox, this));
	mSndLoopChkBox->setCommitCallback(boost::bind(&OSFloaterTools::onClickUpdateChkbox, this));
	mRezLoopChkBox->setCommitCallback(boost::bind(&OSFloaterTools::onClickUpdateChkbox, this));
	mRdmPosChkBox->setCommitCallback(boost::bind(&OSFloaterTools::onClickUpdateChkbox, this));

	// scrolllist callbacks
	//mScrollList->setCommitCallback(boost::bind(&OSFloaterTools::onChooseFromScrollList, this));

	mProfileBtn->setEnabled(FALSE);
	mTextEditor->setEnabled(FALSE);
	mFunctionComboBox->setEnabled(FALSE);
	mSendPacketBtn->setEnabled(FALSE);

	//touch spam
	childSetText("object_name_label", mTouchTimer ? mTouchTimer->getTarget() : "No Target Set For Touch Spam");
	LLButton* pick_btn = getChild<LLButton>("pick_btn");
	if (pick_btn)
	{
		pick_btn->setImages(std::string("UIImgFaceUUID"),
			std::string("UIImgFaceSelectedUUID"));
		childSetAction("pick_btn", onClickObjPicker, this);
	}
	childSetAction("stop_touch_spam", stopTouchSpam, this);
	//end touch spam

	//Set Spinners to Our Pos
	LLVector3d agentPos = gAgent.getPositionGlobal();
	S32 agent_x = llmath::llround((F32)fmod(agentPos.mdV[VX], (F64)REGION_WIDTH_METERS));
	S32 agent_y = llmath::llround((F32)fmod(agentPos.mdV[VY], (F64)REGION_WIDTH_METERS));
	S32 agent_z = llmath::llround((F32)agentPos.mdV[VZ]);
	childSetValue("X", LLSD(agent_x));
	childSetValue("Y", LLSD(agent_y));
	childSetValue("Z", LLSD(agent_z));

	//Follow Prims
	mFollowPrimList = getChild<LLScrollListCtrl>("followprim_list");
	//mFollowPrimList->setCallbackUserData(this);
	mFollowPrimList->sortByColumn("fpl_AvatarName", TRUE);

	refresh();
	tick();
	return TRUE;
}

void OSFloaterTools::refresh()
{
	mFunctionComboBox->setValue((S32)gSavedSettings.getS32("ObjectFunctionType"));
}

BOOL OSFloaterTools::tick()
{
	LLViewerRegion *regionp = gAgent.getRegion();
	if (regionp)
	{
		bool active = childGetValue("loop_chk").asBoolean();
		F32 throttle = (F32)getChild<LLUICtrl>("loop")->getValue().asInteger();

		if (mSoundLoop || mRezLoop)
		{
			F32 throttle = 0.0f;
			if (mRezLoop) throttle = (F32)getChild<LLUICtrl>("LoopSpeed")->getValue().asInteger();
			else if (mSoundLoop) throttle = (F32)getChild<LLUICtrl>("sound_amount")->getValue().asInteger();
			F32 fps = LLViewerStats::getInstance()->mFPSStat.getMeanPerSec();
			F32 x = 0.0f;
			F32 tosend = throttle / fps;
			while (x < tosend)
			{
				new PacketTimer();
				x = x + 1.0f;
			}
		}

		//getChild<LLUICtrl>("region name")->setValue(regionp->getName());
		std::vector<LLUUID> avatar_ids;
		//std::vector<LLUUID> sorted_avatar_ids;
		std::vector<LLVector3d> positions;
		LLVector3d mypos = gAgent.getPositionGlobal();
		LLWorld::instance().getAvatars(&avatar_ids, &positions, mypos, F32_MAX);
		size_t i;
		size_t count = avatar_ids.size();
		for (i = 0; i < count; ++i)
		{
			const LLUUID &avid = avatar_ids[i];

			//filter
			if (avid != mTargetAvatar) break;

			std::string name;
			if (!LLAvatarNameCache::getNSName(avid, name))
				continue; //prevent (Loading...)
			LLVector3d position = positions[i];
			LLVOAvatar* avatarp = gObjectList.findAvatar(avid);
			if (avatarp)
			{
				// Get avatar data
				position = gAgent.getPosGlobalFromAgent(avatarp->getCharacterPosition());
				
				//Check For FollowPrims and Update
				LLUUID checkem = mFollowPrims[avid].getAvatarID();
				if (checkem.notNull())
				{
					U8 data[256];
					U32 FP_LocalID = mFollowPrims[avid].getObjectLocalID();
					LLUUID FP_ID = mFollowPrims[avid].getObjectID();
					LLVector3 FP_AVPOS = avatarp->getCharacterPosition();

					//update position
					LLMessageSystem* msg = gMessageSystem;
					msg->newMessageFast(_PREHASH_MultipleObjectUpdate);
					msg->nextBlockFast(_PREHASH_AgentData);
					msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
					msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
					msg->nextBlockFast(_PREHASH_ObjectData);
					msg->addU32Fast(_PREHASH_ObjectLocalID, FP_LocalID);
					msg->addU8Fast(_PREHASH_Type, U8(0x01));
					htonmemcpy(&data[0], &(FP_AVPOS.mV), MVT_LLVector3, 12);
					msg->addBinaryDataFast(_PREHASH_Data, data, 12);
					msg->sendReliable(gAgent.getRegion()->getHost());
				}

				//Set Spinners to Target Pos
				//LLVector3d agentPos = target_object->getPositionGlobal();
				S32 agent_x = llmath::llround((F32)fmod(position.mdV[VX], (F64)REGION_WIDTH_METERS));
				S32 agent_y = llmath::llround((F32)fmod(position.mdV[VY], (F64)REGION_WIDTH_METERS));
				S32 agent_z = llmath::llround((F32)position.mdV[VZ]);
				childSetValue("X", LLSD(agent_x));
				childSetValue("Y", LLSD(agent_y));
				childSetValue("Z", LLSD(agent_z));

				if (sendTypingStop == TRUE)
				{
					if (active)
					{
						sendTypingStop = TRUE;
						F32 fps = LLViewerStats::getInstance()->mFPSStat.getMeanPerSec();
						F32 x = 0.0f;
						F32 tosend = throttle / fps;
						while (x < tosend)
						{
							new PacketTimer();
							x = x + 1.0f;
						}
					}
					else
					{
						new PacketTimer();
					}
				}
			}
		}
	}

	return FALSE;//keep looping
}

//online status
// virtual
void  OSFloaterTools::processProperties(void* data, EAvatarProcessorType type)
{
	if (type == APT_PROPERTIES)
	{
		const LLAvatarData* pAvatarData = static_cast<const LLAvatarData*>(data);
		if (pAvatarData && (mTargetAvatar == pAvatarData->avatar_id) && (pAvatarData->avatar_id != LLUUID::null))
		{
			bool online = (pAvatarData->flags & AVATAR_ONLINE);

			OnlineStatus online_status = (online) ? ONLINE_STATUS_IS : ONLINE_STATUS_NOT;

			setOnlineStatus(online_status);

		}
	}

}

void OSFloaterTools::setOnlineStatus(OnlineStatus online_status)
{
	// Online status NO could be because they are hidden
	// If they are a friend, we may know the truth!
	if ((ONLINE_STATUS_IS != online_status)
		&& mIsFriend
		&& (LLAvatarTracker::instance().isBuddyOnline(mTargetAvatar)))
	{
		online_status = ONLINE_STATUS_IS;
	}
	childSetVisible("online_yes", online_status == ONLINE_STATUS_IS);
	gSavedSettings.setBOOL("AutoAcceptNewInventory", TRUE);
}
// end online status

void OSFloaterTools::setTargetAvatar(const LLUUID &target_id)
{
	mTargetAvatar = target_id;
	if (target_id.isNull())
	{
		getChild<LLUICtrl>("target_avatar_name")->setValue(getString("no_target"));
	}
}

void OSFloaterTools::onClickSet()
{
	LLFloaterAvatarPicker* picker = LLFloaterAvatarPicker::show(boost::bind(&OSFloaterTools::callbackAvatarID, sInstance, _1, _2));
	// grandparent is a floater, which can have a dependent
	if (picker)
	{
		gFloaterView->getParentFloater(sInstance)->addDependentFloater(picker);
	}
}

void OSFloaterTools::onClickSendPacket()
{
	sendTypingStop = TRUE;
	refresh();
	//new PacketTimer();
}

void OSFloaterTools::onClickProfile()
{
	if (mTargetAvatar.notNull())
	LLAvatarActions::showProfile(mTargetAvatar);
}

void OSFloaterTools::onClickPlaySound()
{
	LLUUID sound = LLUUID(childGetValue("sound_asset_id").asString());
	S32 num_sounds = childGetValue("sound_amount").asInteger();
	LLViewerObject *target_object = NULL;
	LLVector3 pos = gAgent.getPositionAgent();
	S32 sounds_played = 0;

	if (mTargetAvatar.notNull())
	{
		target_object = gObjectList.findObject(mTargetAvatar);
		pos = target_object->getPosition();
	}
	while (sounds_played < num_sounds)
	{
		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_SoundTrigger);
		msg->nextBlockFast(_PREHASH_SoundData);
		msg->addUUIDFast(_PREHASH_SoundID, sound);
		// Client untrusted, ids set on sim
		msg->addUUIDFast(_PREHASH_OwnerID, LLUUID::null);
		msg->addUUIDFast(_PREHASH_ObjectID, LLUUID::null);
		msg->addUUIDFast(_PREHASH_ParentID, LLUUID::null);
		msg->addU64Fast(_PREHASH_Handle, gAgent.getRegion()->getHandle());
		msg->addVector3Fast(_PREHASH_Position, pos);
		msg->addF32Fast(_PREHASH_Gain, 10.f);
		gAgent.sendMessage();
		++sounds_played;
	}
}

void OSFloaterTools::onClickAssetOpen()
{
	LLUUID key = LLUUID(childGetValue("sound_asset_id").asString());
	if (key.notNull())
	{
		LLFloaterKeyTool::show(key);
	}
}

void OSFloaterTools::onClickTpGround()
{
	LLAvatarActions::goToGround();
}

void OSFloaterTools::onClickTpSafe()
{
	LLAvatarActions::goToPanic();
}

void OSFloaterTools::onClickUndeform()
{
	LLDynamicArray<LLUUID> ids;
	ids.put(LLUUID("f097842c-632f-b5c7-bbbd-a0626916437d"));
	ids.put(LLUUID("e5afcabe-1601-934b-7e89-b0c78cac373a"));
	ids.put(LLUUID("d307c056-636e-dda6-4a3c-b3a43c431ca8"));
	ids.put(LLUUID("319b4e7a-18fc-1f9e-6411-dd10326c0c7e"));
	ids.put(LLUUID("f05d765d-0e01-5f9a-bfc2-fdc054757e55"));
	gAgent.sendAnimationRequests(ids, ANIM_REQUEST_START);
}

void OSFloaterTools::onClickBlink()
{
	LLViewerObject* objectp = LLSelectMgr::getInstance()->getSelection()->getFirstRootObject();
	if (objectp)
	{
		LLVector3 pos = objectp->getPosition();
		pos.mV[VZ] = 4294967294.0f;
		//pos.mV[VZ] = 340282346638528859811704183484516925440.0f;
		objectp->setPositionParent(pos);
		LLSelectMgr::getInstance()->sendMultipleUpdate(UPD_POSITION);
	}
}

void OSFloaterTools::onClickDeleteAll()
{
	gObjectList.deleteAllOwnedObjects();
}

void OSFloaterTools::onClickzTake()
{
	LLViewerObject* objectp = LLSelectMgr::getInstance()->getSelection()->getFirstObject();
	const LLUUID& category_id = gInventory.findCategoryUUIDForType(LLFolderType::FT_OBJECT);
	if (!objectp) return;
	LLUUID tid;
	tid.generate();

	gMessageSystem->newMessageFast(_PREHASH_DeRezObject);
	gMessageSystem->nextBlockFast(_PREHASH_AgentData);
	gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	gMessageSystem->nextBlockFast(_PREHASH_AgentBlock);
	gMessageSystem->addUUIDFast(_PREHASH_GroupID, LLUUID::null);
	gMessageSystem->addU8Fast(_PREHASH_Destination, DRD_RETURN_TO_OWNER);
	gMessageSystem->addUUIDFast(_PREHASH_DestinationID, category_id);
	gMessageSystem->addUUIDFast(_PREHASH_TransactionID, tid);
	gMessageSystem->addU8Fast(_PREHASH_PacketCount, 1);
	gMessageSystem->addU8Fast(_PREHASH_PacketNumber, 0);
	gMessageSystem->nextBlockFast(_PREHASH_ObjectData);
	gMessageSystem->addU32Fast(_PREHASH_ObjectLocalID, objectp->getLocalID());
	gMessageSystem->sendReliable(gAgent.getRegionHost());
}

void OSFloaterTools::onClickRezPlat()
{
	cmdline_rezplat();
}

void OSFloaterTools::onClickRez()
{
	S32 num_rez = childGetValue("Amount").asInteger();
	S32 rezzed_items = 0;
	LLVector3 agentPos = gAgent.getPositionAgent();
	F64 local_x = childGetValue("X");
	F64 local_y = childGetValue("Y");
	F64 local_z = childGetValue("Z");
	agentPos.mV[VX] = local_x;
	agentPos.mV[VY] = local_y;
	agentPos.mV[VZ] = local_z;
	if (mRdmPos)
	{
	agentPos.mV[VX] = ll_frand(256.1f) - 1.0f;
	agentPos.mV[VY] = ll_frand(256.1f) - 1.0f;
	agentPos.mV[VZ] = local_z;
	}
	LLViewerInventoryItem* item = (LLViewerInventoryItem*)gInventory.getItem((LLUUID)gSavedPerAccountSettings.getString("RadarRezItem"));
	//if (!item) return;
	if (item) gSavedPerAccountSettings.setString("ToolRezItem", item->getUUID().asString());
	LLViewerRegion* regionp = gAgent.getRegion();
	if (!regionp)
	{
		llwarns << "Couldn't find region to rez object" << LL_ENDL;
		return;
	}
	//make_ui_sound("UISndObjectRezIn");

	//if (regionp
	//	&& (regionp->getRegionFlags() & REGION_FLAGS_SANDBOX))
	//{
	//	LLFirstUse::useSandbox();
	//}
	//BOOL remove_from_inventory = !item->getPermissions().allowCopyBy(gAgent.getID());
	while (rezzed_items < num_rez)
	{
		if (gSavedPerAccountSettings.getString("ToolRezItem") == gAgent.getID().asString())
		{
			LLMessageSystem* msg = gMessageSystem;
			msg->newMessageFast(_PREHASH_ObjectAdd);
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			msg->addUUIDFast(_PREHASH_GroupID, gAgent.getGroupID());
			msg->nextBlockFast(_PREHASH_ObjectData);
			msg->addU8Fast(_PREHASH_PCode, LL_PCODE_VOLUME);
			msg->addU8Fast(_PREHASH_Material, LL_MCODE_METAL);

			msg->addU32Fast(_PREHASH_AddFlags, FLAGS_TEMPORARY_ON_REZ);
			msg->addU32Fast(_PREHASH_AddFlags, FLAGS_USE_PHYSICS);
			//if (agentPos.mV[2] > 4096.0)msg->addU32Fast(_PREHASH_AddFlags, FLAGS_CREATE_SELECTED);
			//else msg->addU32Fast(_PREHASH_AddFlags, 0);
			LLVolumeParams    volume_params;
			volume_params.setType(LL_PCODE_PROFILE_SQUARE, LL_PCODE_PATH_CIRCLE_33);
			volume_params.setRatio(2, 2);
			volume_params.setShear(0, 0);
			volume_params.setTaper(2.0f, 2.0f);
			volume_params.setTaperX(0.f);
			volume_params.setTaperY(0.f);
			LLVolumeMessage::packVolumeParams(&volume_params, msg);
			LLVector3 rezpos = agentPos;
			LLQuaternion rotation;
			rotation.setQuat(90.f * DEG_TO_RAD, LLVector3::y_axis);

			F32 realsize = 1024;
			if (realsize < 0.01f) realsize = 0.01f;
			else if (realsize > 60.0f) realsize = 60.0f;

			msg->addVector3Fast(_PREHASH_Scale, LLVector3(realsize, realsize, realsize));
			msg->addQuatFast(_PREHASH_Rotation, rotation);
			msg->addVector3Fast(_PREHASH_RayStart, rezpos);
			msg->addVector3Fast(_PREHASH_RayEnd, rezpos);
			msg->addU8Fast(_PREHASH_BypassRaycast, (U8)1);
			msg->addU8Fast(_PREHASH_RayEndIsIntersection, (U8)FALSE);
			msg->addU8Fast(_PREHASH_State, 0);
			msg->addUUIDFast(_PREHASH_RayTargetID, LLUUID::null);
			msg->sendReliable(gAgent.getRegionHost());
		}
		else
		{
			if (!item) return;
			//Actual Rez Message
			LLMessageSystem* msg = gMessageSystem;
			BOOL remove_from_inventory = !item->getPermissions().allowCopyBy(gAgent.getID());
			msg->newMessageFast(_PREHASH_RezObject);
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			msg->addUUIDFast(_PREHASH_GroupID, gAgent.getGroupID());

			msg->nextBlock("RezData");
			msg->addUUIDFast(_PREHASH_FromTaskID, LLUUID::null);
			msg->addU8Fast(_PREHASH_BypassRaycast, (U8)TRUE);
			msg->addVector3Fast(_PREHASH_RayStart, agentPos);
			msg->addVector3Fast(_PREHASH_RayEnd, agentPos);
			msg->addUUIDFast(_PREHASH_RayTargetID, LLUUID::null);
			msg->addBOOLFast(_PREHASH_RayEndIsIntersection, FALSE);
			msg->addBOOLFast(_PREHASH_RezSelected, true);
			msg->addBOOLFast(_PREHASH_RemoveItem, remove_from_inventory);
			// deal with permissions slam logic
			pack_permissions_slam(msg, item->getFlags(), item->getPermissions());

			LLUUID folder_id = item->getParentUUID();
			msg->nextBlockFast(_PREHASH_InventoryData);
			item->packMessage(msg);
			msg->sendReliable(regionp->getHost());
		}

		++rezzed_items;
	}
}

void OSFloaterTools::onClickMegaPrim()
{
	gSavedPerAccountSettings.setString("ToolRezItem", "");
	gSavedPerAccountSettings.setString("RadarRezItem", "");
	gSavedPerAccountSettings.setString("ToolRezItem", gAgent.getID().asString());
	refresh();
}

struct OSDuplicateData
{
	LLVector3	offset;
	U32			flags;
};
void OSFloaterTools::onClickDuplicate()
{
	S32 repeats = childGetValue("Amount").asInteger();
	for (S32 x = 0; x < repeats; x++)
	{
		LLVector3 offset = LLVector3(LLVector3::zero);
		OSDuplicateData	data;
		data.offset = offset;
		data.flags = FLAGS_CREATE_SELECTED;
		LLSelectMgr::getInstance()->sendListToRegions("ObjectDuplicate", LLSelectMgr::getInstance()->packDuplicateHeader, LLSelectMgr::getInstance()->packDuplicate, &data, SEND_ONLY_ROOTS);
	}
}

void OSFloaterTools::onClickDerez()
{
	S32 function = gSavedSettings.getS32("ObjectFunctionType");
	EDeRezDestination derez = (EDeRezDestination)function;
	U8 d = (U8)derez;
	LLUUID tid;
	LLUUID dest_id = LLUUID::null;
	LLUUID category_id = gInventory.findCategoryUUIDForType(LLFolderType::FT_OBJECT);
	LLUUID trash_id = gInventory.findCategoryUUIDForType(LLFolderType::FT_TRASH);
	tid.generate();

	if (function == 1 || function == 4 || function == 5 || function == 6 || function == 7)
	{
		dest_id = gInventory.findCategoryUUIDForType(LLFolderType::FT_OBJECT);
	}

	for (LLObjectSelection::valid_iterator iter = LLSelectMgr::getInstance()->getSelection()->valid_begin(); iter != LLSelectMgr::getInstance()->getSelection()->valid_end(); iter++)
	{
		LLSelectNode* obj = *iter;
		if (obj->mCreationDate == 0)
		{
			continue;
		}

		if (obj->getObject()->isRoot())
		{
			if (function == 2)
			{
				if (obj && (obj->mValid) && (!obj->mFromTaskID.isNull()))
				{
					// *TODO: check to see if the fromtaskid object exists.
					dest_id = obj->mFromTaskID;
				}
			}
			LLViewerObject* object = obj->getObject();
			LLMessageSystem* msg = gMessageSystem;
			msg->newMessageFast(_PREHASH_DeRezObject);
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			msg->nextBlockFast(_PREHASH_AgentBlock);
			msg->addUUIDFast(_PREHASH_GroupID, gAgent.getGroupID());
			msg->addU8Fast(_PREHASH_Destination, d);
			msg->addUUIDFast(_PREHASH_DestinationID, dest_id);
			msg->addUUIDFast(_PREHASH_TransactionID, tid);
			msg->addU8Fast(_PREHASH_PacketCount, (U8)1);
			msg->addU8Fast(_PREHASH_PacketNumber, (U8)1);
			msg->nextBlockFast(_PREHASH_ObjectData);
			msg->addU32Fast(_PREHASH_ObjectLocalID, object->getLocalID());
			msg->sendReliable(object->getRegion()->getHost());
			make_ui_sound("UISndObjectRezOut");
			// Busy count decremented by inventory update, so only increment
			// if will be causing an update.
			if (derez != DRD_RETURN_TO_OWNER)
				gViewerWindow->getWindow()->incBusyCount();
		}
	}
}

//######## Follow Prim ########
void OSFloaterTools::onClickAddFollowPrim()
{
	if (mTargetAvatar.isNull())
	{
		std::string chmsg = "[FollowPrim] -> No Target Avatar Set";
		cmdline_printchat(chmsg);
		return;
	}

	std::string fplicon = "------ >";
	//Get current Selection
	LLViewerObject* object = LLSelectMgr::getInstance()->getSelection()->getPrimaryObject();
	if (object)
	{
		if (!object->isAvatar())
		{
			//mTargetAvatar = ids[0];
			//mTargetAvatarName = names[0].getCompleteName();
			if (object->permYouOwner() && object->permModify())
			{
				U32 ObjLocID = object->getLocalID();
				LLUUID ObjID = object->getID();
				//Generate FollowPrim Element
				LLSD element;
				element["id"] = mTargetAvatar;
				element["columns"][LIST_FPLAV_NAME]["color"] = LLColor4::black.getValue();
				element["columns"][LIST_FPLAV_NAME]["column"] = "fpl_AvatarName";
				element["columns"][LIST_FPLAV_NAME]["type"] = "text";
				element["columns"][LIST_FPLAV_NAME]["value"] = mTargetAvatarName.c_str();

				element["columns"][LIST_FPL_ICON]["color"] = LLColor4::black.getValue();
				element["columns"][LIST_FPL_ICON]["column"] = "fpl_icon";
				element["columns"][LIST_FPL_ICON]["type"] = "text";
				element["columns"][LIST_FPL_ICON]["value"] = fplicon.c_str();

				element["columns"][LIST_FPLOBJECTSID]["color"] = LLColor4::black.getValue();
				element["columns"][LIST_FPLOBJECTSID]["column"] = "fpl_ObjectsID";
				element["columns"][LIST_FPLOBJECTSID]["type"] = "text";
				element["columns"][LIST_FPLOBJECTSID]["value"] = ObjID.asString().c_str();

				// Add to list
				mFollowPrimList->addElement(element, ADD_BOTTOM);
				// Sort List
				mFollowPrimList->updateSort();

				//Add all Values to the List
				FollowPrimList entry(mTargetAvatar, mTargetAvatarName, ObjID, ObjLocID);
				mFollowPrims[mTargetAvatar] = entry;

				//deselect and let em go
				LLSelectMgr::getInstance()->deselectAll();
				gFloaterTools->close(true);

			}
			else
			{
				std::string chmsg = "[FollowPrim] -> Missing Rights for that Prim";
				cmdline_printchat(chmsg);
			}
		}
		else
		{
			std::string chmsg = "[FollowPrim] -> Object is an Avatar";
			cmdline_printchat(chmsg);
		}

	}
	else
	{
		std::string chmsg = "[FollowPrim] -> No Prim Selected";
		cmdline_printchat(chmsg);
	}
}

void OSFloaterTools::onClickRemoveFollowPrim()
{
	LLScrollListItem *item = mFollowPrimList->getFirstSelected();
	LLViewerRegion *region = gAgent.getRegion();
	if (item)
	{
		LLUUID user_id = LLUUID(item->getValue());
		U32 FPLLocalID = mFollowPrims[user_id].getObjectLocalID();
		LLUUID FPLPrimID = mFollowPrims[user_id].getObjectID();
		LLViewerObject *obj = gObjectList.findObject(FPLPrimID);
		if (obj)
		{
			gMessageSystem->newMessageFast(_PREHASH_ObjectDelete);
			gMessageSystem->nextBlockFast(_PREHASH_AgentData);
			gMessageSystem->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			gMessageSystem->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			const U8 NO_FORCE = 0;
			gMessageSystem->addU8Fast(_PREHASH_Force, NO_FORCE);
			gMessageSystem->nextBlockFast(_PREHASH_ObjectData);
			gMessageSystem->addU32Fast(_PREHASH_ObjectLocalID, FPLLocalID);
			gMessageSystem->sendReliable(region->getHost());
			mFollowPrimList->deleteSelectedItems();
			mFollowPrims.erase(user_id);
		}
		else
		{
			mFollowPrimList->deleteSelectedItems();
			mFollowPrims.erase(user_id);
		}
	}
}

void OSFloaterTools::callbackAvatarID(const uuid_vec_t& ids, const std::vector<LLAvatarName> names)
{
	if (ids.empty() || names.empty()) return;
	mTargetAvatar = ids[0];
	mTargetAvatarName = names[0].getCompleteName();
	getChild<LLUICtrl>("target_avatar_name")->setValue(mTargetAvatarName);
	getChild<LLUICtrl>("target_avatar_UUID")->setValue(mTargetAvatar.asString());
	//mIsFriend = LLAvatarActions::isFriend(mTargetAvatar); //online status
	LLAvatarPropertiesProcessor::getInstance()->addObserver(mTargetAvatar, this); //online status
	LLAvatarPropertiesProcessor::getInstance()->sendAvatarPropertiesRequest(mTargetAvatar); //online status
	mProfileBtn->setEnabled(TRUE);
	mTextEditor->setEnabled(TRUE);
	mFunctionComboBox->setEnabled(TRUE);
	mSendPacketBtn->setEnabled(TRUE);
	refresh();
}

void OSFloaterTools::onClickUpdateChkbox()
{
	mSoundList = mSoundChkBox->getValue().asBoolean();
	mSoundEcho = mEchoChkBox->getValue().asBoolean();
	mSoundLoop = mSndLoopChkBox->getValue().asBoolean();
	mRezLoop = mRezLoopChkBox->getValue().asBoolean();
	mRdmPos = mRdmPosChkBox->getValue().asBoolean();
	refresh();
}

void OSFloaterTools::onCommitFunctionCombo(LLUICtrl* ctrl)
{
	packetType = atoi(mFunctionComboBox->getValue().asString().c_str());
}

void OSFloaterTools::onCommitSoundCombo(LLUICtrl* ctrl)
{
	getChild<LLUICtrl>("sound_asset_id")->setValue(mSoundComboBox->getValue().asString());
}

void OSFloaterTools::onCommitDerezCombo(LLUICtrl* ctrl)
{
	DerezType = atoi(mDerezComboBox->getValue().asString().c_str());
	gSavedSettings.setS32("ObjectFunctionType", DerezType);
}

void OSFloaterTools::onCommitSetText()
{
	gSavedSettings.setString("OSToolTextEditor", mTextEditor->getText());
}

//touch spam
void OSFloaterTools::onClickObjPicker(void *userdata)
{
	LLToolObjPicker::getInstance()->setExitCallback(OSFloaterTools::closePickTool, sInstance);
	LLToolMgr::getInstance()->setTransientTool(LLToolObjPicker::getInstance());
	LLButton* pick_btn = sInstance->getChild<LLButton>("pick_btn");
	if (pick_btn)pick_btn->setToggleState(TRUE);
}

void OSFloaterTools::closePickTool(void *userdata)
{
	LLUUID spamobject_id = LLToolObjPicker::getInstance()->getObjectID();
	LLViewerObject* spamobject = gObjectList.findObject( spamobject_id );
	if (spamobject)
	{
		if ( spamobject->isAttachment() )
		{
			spamobject = spamobject->getRootEdit();                
		}
		if (!spamobject->isAvatar())
		{
			mTouchTimer = new TouchTimer(spamobject);
			sInstance->childSetText("object_name_label", mTouchTimer->getTarget());
		}
	}
	LLToolMgr::getInstance()->clearTransientTool();
	LLButton* pick_btn = sInstance->getChild<LLButton>("pick_btn");
	if (pick_btn) pick_btn->setToggleState(FALSE);
}

void OSFloaterTools::stopTouchSpam(void *userdata)
{
	if(mTouchTimer)
	{
		delete mTouchTimer;
		mTouchTimer = NULL;
	}
	sInstance->childSetText("object_name_label", LLStringExplicit("No Target Set For Touch Spam"));
}

TouchTimer::TouchTimer(LLViewerObject* object)  : LLEventTimer(0.025f)
{
	myObject = object;
};

TouchTimer::~TouchTimer()
{
	OSFloaterTools::mTouchTimer = NULL;
	myObject = NULL;
}

std::string TouchTimer::getTarget()
{
	if(!myObject)
		return "No Target";
	else
		return myObject->getID().asString();
}

BOOL TouchTimer::tick()
{
	if(!myObject)
		return TRUE;
	if(myObject->isDead())
		return TRUE;
	LLMessageSystem	*msg = gMessageSystem;
	msg->newMessageFast(_PREHASH_ObjectGrab);
	msg->nextBlockFast( _PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	msg->nextBlockFast( _PREHASH_ObjectData);
	msg->addU32Fast(    _PREHASH_LocalID, myObject->mLocalID);
	msg->addVector3Fast(_PREHASH_GrabOffset, LLVector3::zero );
	msg->nextBlock("SurfaceInfo");
	msg->addVector3("UVCoord", LLVector3::zero);
	msg->addVector3("STCoord", LLVector3::zero);
	msg->addS32Fast(_PREHASH_FaceIndex, 0);
	msg->addVector3("Position", myObject->getPosition());
	msg->addVector3("Normal", LLVector3::zero);
	msg->addVector3("Binormal", LLVector3::zero);
	msg->sendMessage( myObject->getRegion()->getHost());

	// *NOTE: Hope the packets arrive safely and in order or else
	// there will be some problems.
	// *TODO: Just fix this bad assumption.
	msg->newMessageFast(_PREHASH_ObjectDeGrab);
	msg->nextBlockFast(_PREHASH_AgentData);
	msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
	msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
	msg->nextBlockFast(_PREHASH_ObjectData);
	msg->addU32Fast(_PREHASH_LocalID, myObject->mLocalID);
	msg->nextBlock("SurfaceInfo");
	msg->addVector3("UVCoord", LLVector3::zero);
	msg->addVector3("STCoord", LLVector3::zero);
	msg->addS32Fast(_PREHASH_FaceIndex, 0);
	msg->addVector3("Position", myObject->getPosition());
	msg->addVector3("Normal", LLVector3::zero);
	msg->addVector3("Binormal", LLVector3::zero);
	msg->sendMessage(myObject->getRegion()->getHost());
	return FALSE;
}


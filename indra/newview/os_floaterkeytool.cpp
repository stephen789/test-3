
#include "llviewerprecompiledheaders.h"

#include "os_floaterkeytool.h"
#include "os_keytool.h"

#include "lluictrlfactory.h"
#include "llviewercontrol.h"
#include "llcommandhandler.h"
#include "os_invtools.h"
#include "lltextbox.h"

std::list<LLFloaterKeyTool *> LLFloaterKeyTool::sInstances;

LLFloaterKeyTool::LLFloaterKeyTool(LLUUID id)
: LLFloater()
{
	sInstances.push_back(this);
	mKey = id;
	LLUICtrlFactory::getInstance()->buildFloater(this, "os_floater_keytool.xml");
}
LLFloaterKeyTool::~LLFloaterKeyTool()
{
	sInstances.remove(this);
	if (mKeyTool) // sanity check: just incase FOR SOME REASON we didn't init mKeyTool
	delete mKeyTool; // this causes KeyTool to shut down, and potentially try to disconnect signals
}

void LLFloaterKeyTool::show(LLUUID id)
{
	S32 left, top;
	gFloaterView->getNewFloaterPosition(&left, &top);
	LLRect rect = gSavedSettings.getRect("FloaterKeyToolRect");
	rect.translate(left - rect.mLeft, top - rect.mTop);
	LLFloaterKeyTool* floaterp = new LLFloaterKeyTool(id);
	floaterp->setRect(rect);
	gFloaterView->adjustToFitScreen(floaterp, FALSE);
}

void LLFloaterKeyTool::close(bool app_quitting)
{
	LLFloater::close(app_quitting);
}

BOOL LLFloaterKeyTool::postBuild()
{
	setIsChrome(TRUE);
	setTitle(std::string("KeyTool"));

	mListBottom = getRect().getHeight() - 45;
	showType(LLKeyTool::KT_AGENT, LLAssetType::AT_NONE, MAYBE);
	showType(LLKeyTool::KT_TASK, LLAssetType::AT_NONE, MAYBE);
	showType(LLKeyTool::KT_GROUP, LLAssetType::AT_NONE, MAYBE);
	showType(LLKeyTool::KT_REGION, LLAssetType::AT_NONE, MAYBE);
	showType(LLKeyTool::KT_PARCEL, LLAssetType::AT_NONE, MAYBE);
	showType(LLKeyTool::KT_ITEM, LLAssetType::AT_NONE, MAYBE);
	showType(LLKeyTool::KT_ASSET, LLAssetType::AT_TEXTURE, MAYBE);
	showType(LLKeyTool::KT_ASSET, LLAssetType::AT_SOUND, MAYBE);
	showType(LLKeyTool::KT_ASSET, LLAssetType::AT_ANIMATION, MAYBE);
	showType(LLKeyTool::KT_ASSET, LLAssetType::AT_LANDMARK, MAYBE);
	showType(LLKeyTool::KT_ASSET, LLAssetType::AT_GESTURE, MAYBE);
	showType(LLKeyTool::KT_ASSET, LLAssetType::AT_CLOTHING, MAYBE);
	showType(LLKeyTool::KT_ASSET, LLAssetType::AT_BODYPART, MAYBE);
	//showType(LLKeyTool::KT_ASSET, LLAssetType::AT_COUNT, MAYBE);

	mKeyTool = new LLKeyTool(mKey, keyToolCallback, this);
	return TRUE;
}

void LLFloaterKeyTool::showType(LLKeyTool::LLKeyType key_type, LLAssetType::EType asset_type, isness result)
{
	std::string name = LLKeyTool::aWhat(key_type, asset_type);
	if ((key_type == LLKeyTool::KT_ASSET) && (asset_type == LLAssetType::AT_COUNT))
		name = "other assets";
	LLTextBox* text = getChild<LLTextBox>(name, FALSE);
	if (!text)
	{
		text = new LLTextBox(name, LLRect(10, mListBottom + 20, getRect().mRight, mListBottom));
		text->setFollowsTop();
		text->setColor(LLColor4::white);
		text->setHoverColor(LLColor4::white);
		mListBottom -= 20;
		addChild(text);
	}
	switch (result)
	{
	case YES:
		LLKeyTool::openKey(mKey, key_type, asset_type);
		this->close(FALSE);
		// if this was optional then this would happen, but we're just gonna close it, lol.
		//text->setColor(LLColor4::green);
		break;
	case NO:
		text->setColor(LLColor4::grey);
		break;
	default:
		text->setColor(LLColor4::white);
		break;
	}
}

//static
void LLFloaterKeyTool::keyToolCallback(LLUUID id, LLKeyTool::LLKeyType key_type, LLAssetType::EType asset_type, BOOL is, void* user_data)
{
	LLFloaterKeyTool* floater = (LLFloaterKeyTool*)user_data;
	if (std::find(sInstances.begin(), sInstances.end(), floater) == sInstances.end())
		return;
	floater->showType(key_type, asset_type, is ? YES : NO);
}

//static
void LLFloaterKeyTool::onClickType(void* user_data)
{
	clickData data = *((clickData*)user_data);
	if (data.floater)
	{
		if (std::find(sInstances.begin(), sInstances.end(), data.floater) == sInstances.end())
			return;
		if (!((data.key_type == LLKeyTool::KT_ASSET) && (data.asset_type == LLAssetType::AT_COUNT)))
			LLKeyTool::openKey(data.floater->mKey, data.key_type, data.asset_type);
	}
}

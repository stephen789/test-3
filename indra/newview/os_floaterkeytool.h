
#ifndef LL_LLFLOATERKEYTOOL_H
#define LL_LLFLOATERKEYTOOL_H

#include "llfloater.h"
#include "os_keytool.h"

class LLFloaterKeyTool : public LLFloater
{
public:
	typedef enum
	{
		YES,
		NO,
		MAYBE
	} isness;
	typedef struct
	{
		LLFloaterKeyTool* floater;
		LLKeyTool::LLKeyType key_type;
		LLAssetType::EType asset_type;
	} clickData;
	LLFloaterKeyTool(LLUUID id);
	static void show(LLUUID id);
	void close(bool app_quitting);
	BOOL postBuild();
	void showType(LLKeyTool::LLKeyType key_type, LLAssetType::EType asset_type, isness result);
	static void keyToolCallback(LLUUID id, LLKeyTool::LLKeyType key_type, LLAssetType::EType asset_type, BOOL is, void* userdata);
	static void onClickType(void* user_data);
	static void sertChromeAgain(LLUICtrl* ctrl, void* user_data);
	LLUUID mKey;
	LLKeyTool* mKeyTool;
	S32 mListBottom;
	static std::list<LLFloaterKeyTool*> sInstances;
private:
	~LLFloaterKeyTool();
};

#endif

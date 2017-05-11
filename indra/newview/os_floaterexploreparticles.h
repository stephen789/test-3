/** 
 * @file os_floaterexploreparticles.h
 * @brief Explores Objects Emmiting Particles and gives you the ability to reverse engineer the scripts or just inspect spam emmiting objects.
 *
 **/
#include "llfloater.h"
#include "lluuid.h"
#include "llviewerinventory.h"
#include "llfloaternamedesc.h"
#include "lltexturectrl.h"
#include "lldynamictexture.h"
#include "llcharacter.h"
#include "llquaternion.h"
#include "llscrolllistctrl.h"

class LLVOAvatar;
class LLViewerJointMesh;

class OSParticleListEntry {
public:
	OSParticleListEntry(const std::string& OwnerName = "", const std::string& ObjectName = "", const LLUUID& TextureID = LLUUID::null);

	std::string getAvatarName();
	void setAvatarName(std::string AvatarName);

	std::string getObjectName();
	void setObjectName(std::string ObjectName);

	LLUUID getTextureID();
	void setTextureID(LLUUID TextureID);

private:
	friend class OSParticleExplorer;

	std::string mOwnerName;
	std::string mObjectName;
	LLUUID mTextureID;
};

struct ObjectDetails
{
	LLUUID id;
	LLUUID Texture;
};

class OSParticleExplorer : public LLFloater
{
public:

	OSParticleExplorer();
	virtual ~OSParticleExplorer();

	BOOL postBuild();
	void draw();
	void close(bool app = false);
	void refresh();
	static OSParticleExplorer* getInstance(){ return sInstance; }
	static void toggle();

	static void onDoubleClick(void *user_data);
	static void onClickReload(void* user_data);
	static void onClickCopy(void* user_data);
	static void onClickReturn(void* user_data);
	static void onClickBeacon(void* user_data);
	static void onSelectObject(LLUICtrl* ctrl, void* user_data);

	static void processObjectProperties(LLMessageSystem* msg, void** user_data);

	LLUUID getSelectedUUID();

private:

	static OSParticleExplorer* sInstance;
	static LLScrollListCtrl* mTheList;
	static LLTextureCtrl* texture_ctrl;
	static std::map<LLUUID, OSParticleListEntry> mParticleObjectList;
	static std::map<LLUUID, ObjectDetails> mRequests;
	
	enum PART_COLUMN_ORDER
	{
		LIST_OBJECT_OWNER,
		LIST_PARTICLE_SOURCE,
		LIST_SOURCE_TEXTURE
	};

};
#ifndef OS_CIRCUITS_H
#define OS_CIRCUITS_H

#include "llfloater.h"
#include "llsingleton.h"
#include "llviewerregion.h"

// --------------------------------------------------------------------------
// RecentRegionList: holds a list of recently connected or disconnected
// regions for use, so we don't have to keep the circuits floater
// loaded all the time.
// --------------------------------------------------------------------------

class OSCircuits;

class RecentRegionList
:	public LLSingleton<RecentRegionList>
{
	friend class LLSingleton<RecentRegionList>;

	private:
		RecentRegionList();
		~RecentRegionList();

	public:
		struct RegionEntry
		{
			std::string name;
			std::string handle;
			LLUUID regionid;
		};

		std::deque<RegionEntry> mRegionList;
		
		void addRegion(const std::string& name, const std::string& handle, const LLUUID& id);	
		void requestList(OSCircuits* circuits);					// request region list
	
		void notifyEnabled(LLViewerRegion* regionp);
		void notifyDisabled(LLViewerRegion* regionp);
		
	private:
	std::map<LLUUID, bool> unique_regionid;
};

// --------------------------------------------------------------------------
// RegionCircuits: floater that shows recently connected or disconnected
// regions, with options to perform various region related tasks.
// --------------------------------------------------------------------------

class LLButton;
class LLCheckBoxCtrl;
class LLScrollListCtrl;
class LLLineEditor;

class OSCircuits : public LLFloater, public LLFloaterSingleton<OSCircuits>
{
	friend class LLUISingleton<OSCircuits, VisibilityPolicy<LLFloater> >;

public:
		/*virtual*/ BOOL postBuild();
		void addRegionList(const std::string& name, const std::string& handle, const LLUUID& regionid);	// called from RecentRegionList
		void update();					// request list update from RecentRegionList
private:
	OSCircuits(const LLSD&);
	~OSCircuits();

protected:
		LLScrollListCtrl* mRegionScrollList;

		LLButton* mDisconnectButton;
		LLButton* mDisconnectAllNeighborsButton;

		LLViewerRegion* mSelectedRegion;// currently selected region
		std::string mSelectedName;		// currently selected region name
		LLUUID mSelectedRegionId;		// currently selected region uuid
		U64 mSelectedRegionhandle;		// currently selected region handle

		LLUUID mRegionId;

		void draw();
		//void update();					// request list update from RecentRegionList
		void onSelectRegion();
		void onDisconnectPressed();
		void onDisconnectAllNeighbors();
};

#endif

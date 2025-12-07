#include "ref_ptr.hpp"
#include "error_macros.hpp"

#include <mutex>
#include <unordered_set>

namespace tst
{
	static std::unordered_set<void *> s_liveReferences;
	static std::mutex                 s_liveReferenceMutex;

	void _addToLiveReferences(void *instance)
	{
		std::scoped_lock<std::mutex> lock(s_liveReferenceMutex);
		TST_ASSERT(instance);
		s_liveReferences.insert(instance);
	}

	void _removeFromLiveReferences(void *instance)
	{
		std::scoped_lock<std::mutex> lock(s_liveReferenceMutex);
		TST_ASSERT(instance);
		TST_ASSERT(s_liveReferences.contains(instance));
		s_liveReferences.erase(instance);
	}

	bool _isLive(void *instance)
	{
		TST_ASSERT(instance);
		return s_liveReferences.contains(instance);
	}
}

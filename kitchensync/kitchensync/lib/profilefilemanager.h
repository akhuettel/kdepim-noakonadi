
#ifndef KSYNC_PROFILE_FILE_MANAGER_H
#define KSYNC_PROFILE_FILE_MANAGER_H

#include "ksync_profile.h"

class KConfig;

namespace KSync {
   /**
    * @internal
    * It's responsible for loading and saving a list
    * of profiles somewhere.... kconfig currently
    *
    */
    class ProfileFileManager {
    public:
        ProfileFileManager();
        ~ProfileFileManager();
        QValueList<Profile> load();
        void save( const QValueList<Profile>& );

    private:
        void saveOne( KConfig* conf, const Profile& prof );
        void saveManPart(KConfig* conf,  const ManPartService&);
        Profile readOne( KConfig* );
        void clear( KConfig* conf );
    };
};

#endif

#ifndef CNOID_BASE_PROJECT_MANAGER_H
#define CNOID_BASE_PROJECT_MANAGER_H

#include "ItemList.h"
#include <string>
#include <functional>
#include "exportdecl.h"

namespace cnoid {

class ExtensionManager;
class Archive;
class Mapping;

class CNOID_EXPORT ProjectManager
{
public:
    // The following functions can be called before the initializeClass function is called.
    static void setDefaultLayoutInclusionMode(bool on);
    [[deprecated("Use setDefaultLayoutInclusionMode")]]
    static void setDefaultOptionToStoreLayoutInProjectFile(bool on);
    static void setTemporaryItemSaveCheckAvailable(bool on);
    
    static void initializeClass(ExtensionManager* ext);
    static ProjectManager* instance() { return instance_; }

    //! The constructor used to create a sub instance for recursive loading / saving
    ProjectManager();
    
    ~ProjectManager();

    void clearProject();

    /*
      \param parentItem The item to add the root item of the project to load.
      If this argument is nullptr, the items of the existing project are cleared
      and the project is newly loaded.
      \return The list of the top level items in the loaded item tree, excluding the root item.
    */
    ItemList<> loadProject(const std::string& filename, Item* parentItem = nullptr);
    
    void loadBuiltinProject(const std::string& resourceFile, Item* parentItem = nullptr);
    bool isLoadingProject() const;
    bool saveProject(const std::string& filename, Item* item = nullptr);
    bool overwriteCurrentProject();
    bool tryToCloseProject();

    bool showDialogToLoadProject();
    bool showDialogToSaveProject();

    const std::string& currentProjectName() const;
    const std::string& currentProjectFile() const;
    const std::string& currentProjectDirectory() const;
    void setCurrentProjectName(const std::string& filename);
    bool isLayoutInclusionMode() const;
    void setLayoutInclusionMode(bool on);

    SignalProxy<void()> sigProjectCleared();

    /**
       This signal is emitted when a project is about to be loaded.
       \note The recursiveLevel argument is 0 for a top level project, and the level
       is recursively incrimented for a sub project contained in the top level project.
       Note that the level starts at 1 even for a top level project when the project
       is loaded below an existing item, 
    */
    SignalProxy<void(int recursiveLevel)> sigProjectAboutToBeLoaded();

    /**
       This signal is emitted when a project has been loaded. The recursiveLevel argument
       is same as that of sigProjectAboutToBeLoaded.
    */
    SignalProxy<void(int recursiveLevel)> sigProjectLoaded();

    ref_ptr<Mapping> storeCurrentLayout();
    void restoreLayout(Mapping* layout);

    bool checkValidItemExistence() const;
    bool checkIfItemsConsistentWithProjectArchive() const;

    class Impl;

private:
    ProjectManager(ExtensionManager* ext);

    Impl* impl;

    friend class ExtensionManager;
    friend class ExtensionManagerImpl;

    void setArchiver(
        const std::string& moduleName,
        const std::string& objectName,
        std::function<bool(Archive&)> storeFunction,
        std::function<void(const Archive&)> restoreFunction);

    void resetArchivers(const std::string& moduleName);

    static ProjectManager* instance_;
};

}

#endif

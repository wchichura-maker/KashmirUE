"""Editor-only bootstrap script for the empty KashmirUE prototype map."""

import unreal

MAP_PATH = "/Game/KashmirAct/Maps/L_Prototype"

if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
    unreal.log("KASHMIR_PROTOTYPE_MAP_EXISTS")
else:
    unreal.EditorLevelLibrary.new_level(MAP_PATH)
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unable to save prototype map")
    unreal.log("KASHMIR_PROTOTYPE_MAP_CREATED")

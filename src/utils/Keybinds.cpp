#include <Geode/Geode.hpp>
#include <geode.custom-keybinds/include/OptionalAPI.hpp>

using namespace geode::prelude;
using namespace keybinds;

$execute {
    (void)[&]() -> Result<> {
        GEODE_UNWRAP(BindManagerV2::registerBindable(GEODE_UNWRAP(BindableActionV2::create(
            "script-menu"_spr,
            "Open Script Menu",
            "Opens the script selector/console menu when in the editor",
            {GEODE_UNWRAP(KeybindV2::create(KEY_B, ModifierV2::Control))},
            GEODE_UNWRAP(CategoryV2::EDITOR)
        ))));
        return Ok();
    }();
}
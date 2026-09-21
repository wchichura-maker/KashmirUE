#include "KashmirGameMode.h"
#include "KashmirCharacter.h"

AKashmirGameMode::AKashmirGameMode()
{
    DefaultPawnClass = AKashmirCharacter::StaticClass();
}

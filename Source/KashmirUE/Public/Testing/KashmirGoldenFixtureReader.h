#pragma once

#include "CoreMinimal.h"

class FJsonObject;

struct FKashmirGoldenFixtureCase
{
    FString Id;
    FString Family;
    TSharedPtr<FJsonObject> Input;
    TSharedPtr<FJsonObject> Expected;
};

struct FKashmirGoldenFixtureDocument
{
    FString SchemaVersion;
    TArray<FKashmirGoldenFixtureCase> Fixtures;
};

class KASHMIRUE_API FKashmirGoldenFixtureReader
{
public:
    static bool LoadDefault(FKashmirGoldenFixtureDocument& OutDocument, FString& OutError);
    static bool LoadFromFile(const FString& Filename, FKashmirGoldenFixtureDocument& OutDocument, FString& OutError);
};

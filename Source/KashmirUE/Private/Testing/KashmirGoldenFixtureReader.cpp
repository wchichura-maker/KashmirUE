#include "Testing/KashmirGoldenFixtureReader.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool FKashmirGoldenFixtureReader::LoadDefault(FKashmirGoldenFixtureDocument& OutDocument, FString& OutError)
{
    return LoadFromFile(
        FPaths::Combine(FPaths::ProjectContentDir(), TEXT("KashmirAct/Data/GoldenFixtures/KashmirGoldenFixtures_v1.json")),
        OutDocument,
        OutError
    );
}

bool FKashmirGoldenFixtureReader::LoadFromFile(const FString& Filename, FKashmirGoldenFixtureDocument& OutDocument, FString& OutError)
{
    OutDocument = {};
    OutError.Reset();
    FString Source;
    if (!FFileHelper::LoadFileToString(Source, *Filename))
    {
        OutError = FString::Printf(TEXT("Fixture file not found: %s"), *Filename);
        return false;
    }
    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Source);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        OutError = TEXT("Fixture document is not valid JSON");
        return false;
    }
    if (!Root->TryGetStringField(TEXT("schema_version"), OutDocument.SchemaVersion) || OutDocument.SchemaVersion.IsEmpty())
    {
        OutError = TEXT("Fixture document is missing schema_version");
        return false;
    }
    const TArray<TSharedPtr<FJsonValue>>* FixtureValues = nullptr;
    if (!Root->TryGetArrayField(TEXT("fixtures"), FixtureValues) || FixtureValues == nullptr || FixtureValues->IsEmpty())
    {
        OutError = TEXT("Fixture document is missing fixtures");
        return false;
    }
    TSet<FString> SeenIds;
    for (const TSharedPtr<FJsonValue>& Value : *FixtureValues)
    {
        const TSharedPtr<FJsonObject> Fixture = Value.IsValid() ? Value->AsObject() : nullptr;
        if (!Fixture.IsValid())
        {
            OutError = TEXT("Fixture entry must be an object");
            return false;
        }
        FKashmirGoldenFixtureCase Entry;
        const TSharedPtr<FJsonObject>* Input = nullptr;
        const TSharedPtr<FJsonObject>* Expected = nullptr;
        if (!Fixture->TryGetStringField(TEXT("id"), Entry.Id) || Entry.Id.IsEmpty()
            || !Fixture->TryGetStringField(TEXT("family"), Entry.Family) || Entry.Family.IsEmpty()
            || !Fixture->TryGetObjectField(TEXT("input"), Input) || Input == nullptr || !Input->IsValid()
            || !Fixture->TryGetObjectField(TEXT("expected"), Expected) || Expected == nullptr || !Expected->IsValid())
        {
            OutError = TEXT("Fixture requires id, family, input and expected");
            return false;
        }
        if (SeenIds.Contains(Entry.Id))
        {
            OutError = FString::Printf(TEXT("Duplicate fixture id: %s"), *Entry.Id);
            return false;
        }
        SeenIds.Add(Entry.Id);
        Entry.Input = *Input;
        Entry.Expected = *Expected;
        OutDocument.Fixtures.Add(MoveTemp(Entry));
    }
    return true;
}

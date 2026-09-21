#if WITH_DEV_AUTOMATION_TESTS

#include "Contracts/KashmirActionContracts.h"
#include "Misc/AutomationTest.h"
#include "Testing/KashmirGoldenFixtureReader.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKashmirActionRequestContractTest,"Kashmir.Foundation.Contracts.ActionRequest",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKashmirActionRequestContractTest::RunTest(const FString&) { FString Reason; FKashmirActionRequest Valid; Valid.ActionId=TEXT("cut_down"); TestTrue(TEXT("named non-negative action is valid"),Valid.IsValid(Reason)); Valid.Intensity=-.01f; TestFalse(TEXT("negative intensity invalid"),Valid.IsValid(Reason)); TestEqual(TEXT("portable invalid reason"),Reason,FString(TEXT("intensity cannot be negative"))); return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKashmirGoldenFixturesTest,"Kashmir.Foundation.Fixtures.Load",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKashmirGoldenFixturesTest::RunTest(const FString&) { FKashmirGoldenFixtureDocument Document; FString Error; TestTrue(TEXT("fixtures load"),FKashmirGoldenFixtureReader::LoadDefault(Document,Error)); TestEqual(TEXT("fixture schema"),Document.SchemaVersion,FString(TEXT("1.0.0"))); TestEqual(TEXT("fixture count"),Document.Fixtures.Num(),14); TSet<FString> Families; for(const auto& Fixture:Document.Fixtures) Families.Add(Fixture.Family); TestTrue(TEXT("action fixtures"),Families.Contains(TEXT("action"))); TestTrue(TEXT("resource fixtures"),Families.Contains(TEXT("resource"))); TestTrue(TEXT("runtime fixtures"),Families.Contains(TEXT("runtime"))); TestTrue(TEXT("hit fixtures"),Families.Contains(TEXT("hit_defense"))); TestTrue(TEXT("interaction fixtures"),Families.Contains(TEXT("interaction"))); TestTrue(TEXT("telemetry fixtures"),Families.Contains(TEXT("telemetry"))); return true; }
#endif

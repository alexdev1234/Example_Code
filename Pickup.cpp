#include "World/Pickup.h"
#include "Items/ItemBase.h"
#include "UserInterface/Inventory/Component/InventoryComponent.h"

// Sets default values
APickup::APickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
	PickupMesh->SetSimulatePhysics(true);
	SetRootComponent(PickupMesh);
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	
	InitializePickup(ItemQuantity);
}

void APickup::InitializePickup(const int32 Quantity)
{
	// Make sure data table and item name are valid
	if (!ItemRowHandle.IsNull())
	{
		const FItemData* ItemData = ItemRowHandle.GetRow<FItemData>(ItemRowHandle.RowName.ToString());

		ItemReference = NewObject<UItemBase>(this, UItemBase::StaticClass());
		ItemReference->ItemData = *ItemData;

		Quantity <= 0 ? ItemReference->SetQuantity(1) : ItemReference->SetQuantity(Quantity);

		PickupMesh->SetStaticMesh(ItemData->ItemAssetData.Mesh);

		UpdateInteractableData();
	}
}

void APickup::InitializeDrop(UItemBase* Item, const int32 Quantity)
{
	if (Item)
	{
		ItemReference = Item;
		Quantity <= 0 ? ItemReference->SetQuantity(1) : ItemReference->SetQuantity(Quantity);

		ItemReference->ItemData.ItemNumeric.Weight = Item->GetItemSingleWeight();
		PickupMesh->SetStaticMesh(Item->ItemData.ItemAssetData.Mesh);

		UpdateInteractableData();
	}
}

void APickup::BeginFocus()
{
	if (PickupMesh)
	{
		PickupMesh->SetRenderCustomDepth(true);
	}
}

void APickup::EndFocus()
{
	if (PickupMesh)
	{
		PickupMesh->SetRenderCustomDepth(false);
	}
}

void APickup::Interact(AStealthProjectCharacter* PlayerCharacter)
{
	if (PlayerCharacter)
	{
		TakePickup(PlayerCharacter);
	}
}

void APickup::UpdateInteractableData()
{
	InstanceInteractableData.InteractableType = EInteractableType::Pickup;
	InstanceInteractableData.Name = ItemReference->ItemData.ItemText.Name;
	InstanceInteractableData.Action = ItemReference->ItemData.ItemText.InteractionText;
	InstanceInteractableData.Quantity = ItemReference->Quantity;
	InteractableData = InstanceInteractableData;
}

void APickup::TakePickup(const AStealthProjectCharacter* Player)
{
	// Only run if the item is not in the process of being destroyed
	if (!IsPendingKillPending())
	{
		if (ItemReference)
		{
			if (UInventoryComponent* PlayerInventory = Player->GetInventory())
			{
				const FItemAddResult AddResult = PlayerInventory->HandleAddItem(ItemReference);

				switch (AddResult.Result)
				{
				case EItemAddResult::NoItemAdded:
					// Don't need to do anything
					break;

				case EItemAddResult::PartialAmountItemAdded:
					UpdateInteractableData();
					Player->UpdateInteractionWidget();
					break;

				case EItemAddResult::AllItemAdded:

					Destroy();
					break;
				}

				UE_LOG(LogTemp, Warning, TEXT("%s"), *AddResult.ResultMessage.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("PLAYER INVENTORY COMPONENT IS NULL."));
			}

		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PICKUP ITEM REFERENCE IS NULL."));
		}
	}
}

#if WITH_EDITOR
// This is not working, need to figure out why
void APickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName ChangedPropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(FDataTableRowHandle, RowName))
	{
		if (!ItemRowHandle.IsNull())
		{
			const FItemData* ItemData = ItemRowHandle.GetRow<FItemData>(ItemRowHandle.RowName.ToString());
			PickupMesh->SetStaticMesh(ItemData->ItemAssetData.Mesh);
		}
	}
}
#endif

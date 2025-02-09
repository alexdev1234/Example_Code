#include "World/Pickup.h"
#include "Items/ItemBase.h"
#include "UserInterface/Inventory/Component/InventoryComponent.h"

// Sets default values
APickup::APickup()
{
	// No need to have this tick every frame
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

		// Make sure the item is properly reflecting the data from the data table
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
		// If the item is removed from the player inventory and dropped back into the world,
		// we want to make sure that all that data is correct.
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
		// Applying custom shader to the item to show it can be picked up
		PickupMesh->SetRenderCustomDepth(true);
	}
}

void APickup::EndFocus()
{
	if (PickupMesh)
	{
		// Removing custom shader since player is no longer looking at it
		PickupMesh->SetRenderCustomDepth(false);
	}
}

void APickup::Interact(AStealthProjectCharacter* PlayerCharacter)
{
	if (PlayerCharacter)
	{
		// Have the pickup be added to the player inventory
		TakePickup(PlayerCharacter);
	}
}

void APickup::UpdateInteractableData()
{
	// Update all necessary data
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

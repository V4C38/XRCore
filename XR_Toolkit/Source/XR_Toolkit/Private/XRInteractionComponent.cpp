
#include "XRInteractionComponent.h"
#include "XRInteractorComponent.h"
#include "GameFramework/GameSession.h"
#include "Kismet/GameplayStatics.h"

UXRInteractionComponent::UXRInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = true;
	SetIsReplicated(true);
}


void UXRInteractionComponent::InitializeComponent()
{
	Super::InitializeComponent();
	GetOwner()->SetReplicates(true);
}

void UXRInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	CacheInteractionCollision();
}

void UXRInteractionComponent::Client_StartInteraction(UXRInteractorComponent* InInteractor)
{
	SetActiveInteractor(InInteractor);
	OnLocalInteractionStart(InInteractor);
	OnInteractionStarted.Broadcast(this, InInteractor);
	if (GetIsContinuousInteraction())
	{
		bIsInteractionActive = true;
	}
}
void UXRInteractionComponent::Client_EndInteraction(UXRInteractorComponent* InInteractor)
{
	OnLocalInteractionEnd(InInteractor);
	OnInteractionEnded.Broadcast(this, InInteractor);
	SetActiveInteractor(nullptr);
	
	UXRInteractorComponent* HoveringInteractor = GetHoveringInteractor();
	if (HoveringInteractor)
	{
		OnInteractionHovered.Broadcast(this, HoveringInteractor, true);
	}
	
	bIsInteractionActive = false;
}

void UXRInteractionComponent::CacheInteractionCollision()
{
	const AActor* ParentActor = GetOwner();
	ParentActor->GetComponents(InteractionCollision);
	
	for (auto* Collider : InteractionCollision)
	{
		if (Collider)
		{
			Collider->OnComponentBeginOverlap.AddDynamic(this, &UXRInteractionComponent::OnOverlapBegin);
			Collider->OnComponentEndOverlap.AddDynamic(this, &UXRInteractionComponent::OnOverlapEnd);
		}
	}
}

TArray<UMeshComponent*> UXRInteractionComponent::GetInteractionCollision() const
{
	return InteractionCollision;
}

UXRInteractorComponent* UXRInteractionComponent::GetHoveringInteractor()
{
	UXRInteractorComponent* OverlappingInteractor = nullptr;
	for (auto* Collider : GetInteractionCollision())
	{
		if (Collider)
		{
			TArray<UPrimitiveComponent*> OverlappingComponents;
			Collider->GetOverlappingComponents(OverlappingComponents);
			for(auto* OverlappingComp : OverlappingComponents)
			{
				if (OverlappingComp->ComponentHasTag("Interactor"))
				{
					OverlappingInteractor = Cast<UXRInteractorComponent>(OverlappingComp);
				}
			}
		}
	}
	return OverlappingInteractor;
}

void UXRInteractionComponent::SetActiveInteractor(UXRInteractorComponent* InInteractor)
{
	if (GetIsContinuousInteraction())
	{
		ActiveInteractor = InInteractor;
	}
}

int32 UXRInteractionComponent::GetInteractionPriority() const
{
	return InteractionPriority;
}

UXRInteractorComponent* UXRInteractionComponent::GetActiveInteractor()
{
	return ActiveInteractor;
}

bool UXRInteractionComponent::GetIsInteractionActive() const
{
	return bIsInteractionActive;
}

bool UXRInteractionComponent::GetAllowTakeOver() const
{
	return bAllowTakeOver;
}

void UXRInteractionComponent::SetAllowTakeOver(bool bInAllowTakeOver)
{
	bAllowTakeOver = bInAllowTakeOver;
}


bool UXRInteractionComponent::GetIsContinuousInteraction() const
{
	return bIsContinuousInteraction;
}

bool UXRInteractionComponent::IsLaserInteractionEnabled() const
{
	return bEnableLaserInteraction;
}

bool UXRInteractionComponent::GetSupressLaserWhenInteracting() const
{
	return bSupressLaserWhenInteracting;
}

void UXRInteractionComponent::SetSupressLaserWhenInteracting(bool InSupressLaser)
{
	bSupressLaserWhenInteracting = InSupressLaser;
}

bool UXRInteractionComponent::GetSnapXRLaserToActor() const
{
	return bSnapXRLaserToActor;
}

void UXRInteractionComponent::SetSnapXRLaserToActor(bool InSnapXRLaserToActor)
{
	bSnapXRLaserToActor = InSnapXRLaserToActor;
}

void UXRInteractionComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp->ComponentHasTag("Interactor"))
	{
		UXRInteractorComponent* XRInteractor = Cast<UXRInteractorComponent>(OtherComp);
		if (XRInteractor)
		{
			OnInteractionHovered.Broadcast(this, XRInteractor,true);
			OnLocalInteractionHovered(true, XRInteractor);
		}
	}
}

void UXRInteractionComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherComp->ComponentHasTag("Interactor"))
	{
		UXRInteractorComponent* XRInteractor = Cast<UXRInteractorComponent>(OtherComp);
		if (XRInteractor)
		{
			OnInteractionHovered.Broadcast(this, XRInteractor,false);
			OnLocalInteractionHovered(false, XRInteractor);
		}
	}
}


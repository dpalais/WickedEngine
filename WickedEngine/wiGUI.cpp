#include "wiGUI.h"
#include "wiWidget.h"
#include "wiHashString.h"
#include "wiRenderer.h"
#include "wiInput.h"

using namespace std;
using namespace wiGraphics;

wiGUI::wiGUI() : activeWidget(nullptr), focus(false), visible(true), pointerpos(XMFLOAT2(0,0))
{
	SetDirty();
	scale_local.x = (float)wiRenderer::GetDevice()->GetScreenWidth();
	scale_local.y = (float)wiRenderer::GetDevice()->GetScreenHeight();
	UpdateTransform();
}


wiGUI::~wiGUI()
{
}


void wiGUI::Update(float dt)
{
	if (!visible)
	{
		return;
	}

	if (wiRenderer::GetDevice()->ResolutionChanged())
	{
		SetDirty();
		scale_local.x = (float)wiRenderer::GetDevice()->GetScreenWidth();
		scale_local.y = (float)wiRenderer::GetDevice()->GetScreenHeight();
		UpdateTransform();
	}

	XMFLOAT4 _p = wiInput::GetPointer();
	pointerpos.x = _p.x;
	pointerpos.y = _p.y;

	if (activeWidget != nullptr)
	{
		if (!activeWidget->IsEnabled() || !activeWidget->IsVisible())
		{
			// deactivate active widget if it became invisible or disabled
			DeactivateWidget(activeWidget);
		}
	}

	focus = false;
	for (auto& widget : widgets)
	{
		if (widget->parent == this)
		{
			// the contained child widgets will be updated by the containers
			widget->Update(this, dt);
		}

		if (widget->IsEnabled() && widget->IsVisible() && widget->GetState() > wiWidget::WIDGETSTATE::IDLE)
		{
			focus = true;
		}
	}

	scissorRect.bottom = (int32_t)(wiRenderer::GetDevice()->GetScreenHeight());
	scissorRect.left = (int32_t)(0);
	scissorRect.right = (int32_t)(wiRenderer::GetDevice()->GetScreenWidth());
	scissorRect.top = (int32_t)(0);
}

void wiGUI::Render(CommandList cmd) const
{
	if (!visible)
	{
		return;
	}

	wiRenderer::GetDevice()->EventBegin("GUI", cmd);
	for (auto&x : widgets)
	{
		if (x->parent == this && x != activeWidget)
		{
			// the contained child widgets will be rendered by the containers
			wiRenderer::GetDevice()->BindScissorRects(1, &scissorRect, cmd);
			x->Render(this, cmd);
		}
	}
	if (activeWidget != nullptr)
	{
		// render the active widget on top of everything
		wiRenderer::GetDevice()->BindScissorRects(1, &scissorRect, cmd);
		activeWidget->Render(this, cmd);
	}

	wiRenderer::GetDevice()->BindScissorRects(1, &scissorRect, cmd);

	for (auto&x : widgets)
	{
		x->RenderTooltip(this, cmd);
	}

	wiRenderer::GetDevice()->EventEnd(cmd);
}

void wiGUI::AddWidget(wiWidget* widget)
{
	widget->AttachTo(this);
	widgets.push_back(widget);
}

void wiGUI::RemoveWidget(wiWidget* widget)
{
	widget->Detach();
	widgets.remove(widget);
}

wiWidget* wiGUI::GetWidget(const wiHashString& name)
{
	for (auto& x : widgets)
	{
		if (x->GetName() == name)
		{
			return x;
		}
	}
	return nullptr;
}

void wiGUI::ActivateWidget(wiWidget* widget)
{
	activeWidget = widget;
	activeWidget->Activate();
}
void wiGUI::DeactivateWidget(wiWidget* widget)
{
	widget->Deactivate();
	if (activeWidget == widget)
	{
		activeWidget = nullptr;
	}
}
const wiWidget* wiGUI::GetActiveWidget() const
{
	return activeWidget;
}
bool wiGUI::IsWidgetDisabled(wiWidget* widget)
{
	return (activeWidget != nullptr && activeWidget != widget);
}
bool wiGUI::HasFocus()
{
	if (!visible)
	{
		return false;
	}

	return focus;
}

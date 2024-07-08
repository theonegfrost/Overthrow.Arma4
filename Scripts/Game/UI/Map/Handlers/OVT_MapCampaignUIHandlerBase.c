[BaseContainerProps()]
class OVT_MapCampaignUIHandlerBase : OVT_MapCampaignUIHandler
{
	override void ShowInfo(Widget layoutRoot)
	{
		OVT_BaseData base = OVT_BaseData.Cast(m_Data);
		
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		
		ImageWidget img = ImageWidget.Cast(layoutRoot.FindAnyWidget("ControllingFaction"));
		img.LoadImageTexture(0, base.ControllingFactionData().GetUIInfo().GetIconPath());
		
		TextWidget widget = TextWidget.Cast(layoutRoot.FindAnyWidget("BaseName"));
		widget.SetText(base.name);		
		
		widget = TextWidget.Cast(layoutRoot.FindAnyWidget("Distance"));
		float distance = vector.Distance(base.location, player.GetOrigin());
		string dis, units;
		SCR_Global.GetDistForHUD(distance, false, dis, units);
		widget.SetText(dis + " " + units);
	}
}
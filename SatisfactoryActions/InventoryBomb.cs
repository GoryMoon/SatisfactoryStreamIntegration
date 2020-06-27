using System.ComponentModel;
using Newtonsoft.Json;

namespace SatisfactoryActions
{
    public class InventoryBomb: BaseAction<InventoryBomb>
    {
        [DefaultValue(2)]
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Populate, PropertyName = "spread")]
        private int _spread;
    }
}
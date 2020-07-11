﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using Newtonsoft.Json;
using StreamIntegrationApp.API;

namespace SatisfactoryActions
{
    public abstract class BaseAction<T>: IntegrationAction, ICloneable where T : BaseAction<T>
    {
        [JsonProperty(PropertyName = "from")]
        private string _from;
        
        [DefaultValue("self")]
        [JsonProperty(PropertyName = "target", DefaultValueHandling = DefaultValueHandling.Populate)]
        private string _target;

        [DefaultValue(0f)]
        [JsonProperty(PropertyName = "delay_min", DefaultValueHandling = DefaultValueHandling.Populate)]
        private float _delayMin;
        
        [DefaultValue(0f)]
        [JsonProperty(PropertyName = "delay_max", DefaultValueHandling = DefaultValueHandling.Populate)]
        private float _delayMax;
        
        [DefaultValue(false)]
        [JsonProperty(PropertyName = "silent", DefaultValueHandling = DefaultValueHandling.Populate)]
        private string _silent;
        
        public override string Execute(string username, string from, Dictionary<string, object> parameters)
        {
            return JsonConvert.SerializeObject(Process((T) Clone(), username, from, parameters));
        }

        protected virtual T Process(T action, string username, string from, Dictionary<string, object> parameters)
        {
            action._from = from;
            return action;
        }

        public object Clone()
        {
            return MemberwiseClone();
        }
    }
}
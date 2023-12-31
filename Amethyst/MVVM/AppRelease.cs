using System;
using System.Collections.Generic;

namespace Amethyst.MVVM;

internal class AppRelease
{
    public string app_name { get; set; }
    public string app_display_name { get; set; }
    public string app_os { get; set; }
    public string app_icon_url { get; set; }
    public string release_notes_url { get; set; }
    public Owner owner { get; set; }
    public bool is_external_build { get; set; }
    public string origin { get; set; }
    public int id { get; set; }
    public Version version { get; set; }
    public string short_version { get; set; }
    public int size { get; set; }
    public string min_os { get; set; }
    public object device_family { get; set; }
    public string bundle_identifier { get; set; }
    public string fingerprint { get; set; }
    public DateTime uploaded_at { get; set; }
    public string download_url { get; set; }
    public string install_url { get; set; }
    public bool mandatory_update { get; set; }
    public bool enabled { get; set; }
    public string fileExtension { get; set; }
    public bool is_latest { get; set; }
    public string release_notes { get; set; }
    public List<string> package_hashes { get; set; }
    public string destination_type { get; set; }
    public string status { get; set; }
    public string distribution_group_id { get; set; }
    public List<DistributionGroups> distribution_groups { get; set; }

    internal class Owner
    {
        public string name { get; set; }
        public string display_name { get; set; }
    }

    public class DistributionGroups
    {
        public string id { get; set; }
        public string name { get; set; }
        public string origin { get; set; }
        public string display_name { get; set; }
        public bool is_public { get; set; }
    }
}

internal class NoticeInfo
{
    public string Title { get; set; } = "";
    public string Content { get; set; } = "";
    public string ButtonText { get; set; } = null;
    public string ButtonLink { get; set; } = null;
    public bool Closable { get; set; } = true;
    public int Severity { get; set; } = 0;

    public bool IsValid => !string.IsNullOrEmpty(Title);
}
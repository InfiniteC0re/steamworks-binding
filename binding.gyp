{
    "targets": [
        {
            "target_name": "steamworks-binding",
            "libraries": ["../dlls/steam_api64.lib", "../dlls/steam_api.lib"],
            "include_dirs": ["./headers/"],
            "sources": ["addon.cc"],
            "conditions": [
                ["OS==\"win\"", {
                    "copies": [
                        {
                            "destination": "<(module_root_dir)/build/Release/",
                            "files": ["<(module_root_dir)/dlls/steam_api64.dll", "<(module_root_dir)/dlls/steam_api.dll"]
                        }
                    ]
                }]
            ]
        }
    ]
}

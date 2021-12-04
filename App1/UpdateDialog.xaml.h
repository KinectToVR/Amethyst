#pragma once

#include "UpdateDialog.g.h"

namespace winrt::App1::implementation
{
    struct UpdateDialog : UpdateDialogT<UpdateDialog>
    {
        UpdateDialog();
    };
}

namespace winrt::App1::factory_implementation
{
    struct UpdateDialog : UpdateDialogT<UpdateDialog, implementation::UpdateDialog>
    {
    };
}

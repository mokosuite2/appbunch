namespace Mokosuite {

    namespace Panel {
        [CCode (cheader_filename = "panel.h")]

        [CCode (cname="mokopanel_notification_queue")]
        public int notification_queue(void* panel_ptr, string? text, string icon, int type, int flags);

        [CCode (cname="mokopanel_notification_remove")]
        public void notification_remove(void* panel_ptr, int notification_id);

        namespace NotificationFlags {
            [CCode (cname = "MOKOPANEL_NOTIFICATION_FLAG_NONE")]
            public const int NONE;

            [CCode (cname = "MOKOPANEL_NOTIFICATION_FLAG_DONT_PUSH")]
            public const int DONT_PUSH;
        }
    }

}


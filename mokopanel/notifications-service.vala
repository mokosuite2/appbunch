/**
 * Implementazione di un servizio di notifiche per mokopanel
 */

namespace Mokosuite
{

	[DBus (name = "org.mokosuite.Notifications")]
    public class NotificationsService : Object
    {
        private void* panel;

		public NotificationsService(DBus.Connection system_bus, string dbus_path, void* panel_ptr)
		{
            this.panel = panel_ptr;
	        system_bus.register_object (dbus_path, this);
        }

		/* === D-BUS API === */

        /**
         * Aggiunge una notifica in coda e ne restituisce l'ID per una futura rimozione.
         * Se il testo è NULL, la notifica è inserita solamente in prima pagina.
         * Se il testo è diverso da NULL, la notifica sarà visualizzata per qualche
         * secondo, riga per riga; dopodiché sarà inserita in prima pagina.
         * 
         * @param text testo della notifica, oppure NULL.
         * @param icon il nome del file dell'icona da visualizza.
         * @param type il tipo della notifica; se l'icona e' gia' presente per questo tipo, non ne sara' aggiunta un'altra
         * @param subdescription sotto-descrizione della notifica (NULL non permesso)
         * @param flags flag per la notifica
         * @return l'ID univoco della notifica
         */
        public int PushNotification(string? text, string type, string subdescription, int flags)
        {
            return Panel.notification_queue(this.panel, text, type, subdescription, flags);
        }

        public void RemoveNotification(int notification_id)
        {
            Panel.notification_remove(this.panel, notification_id);
        }

        public void RegisterType(string type, string icon, string description1, string description2, bool format_count)
        {
            Panel.register_notification_type(this.panel, type, icon, description1, description2, format_count);
        }
    }

}

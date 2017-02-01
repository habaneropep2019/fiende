<!-- ***************************************************************** -->
<sect> Gtk-- Signals <label id="sec_GTK_Signals">
<!-- ***************************************************************** -->
<p>
As Gtk-- is an object oriented widget set, it has a hierarchy of
inheritance. This inheritance mechanism applies for
signals. Therefore, you should refer to the widget hierarchy tree when
using the signals listed in this section.

<!-- ----------------------------------------------------------------- -->
<sect1>GtkObject
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkObject::destroy	(GtkObject *,
                       	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkWidget
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>

void GtkWidget::show	(GtkWidget *,
                    	 gpointer);
void GtkWidget::hide	(GtkWidget *,
                    	 gpointer);
void GtkWidget::map	(GtkWidget *,
                   	 gpointer);
void GtkWidget::unmap	(GtkWidget *,
                     	 gpointer);
void GtkWidget::realize	(GtkWidget *,
                       	 gpointer);
void GtkWidget::unrealize	(GtkWidget *,
                         	 gpointer);
void GtkWidget::draw	(GtkWidget *,
                    	 ggpointer,
                    	 gpointer);
void GtkWidget::draw-focus	(GtkWidget *,
                          	 gpointer);
void GtkWidget::draw-default	(GtkWidget *,
                            	 gpointer);
void GtkWidget::size-request	(GtkWidget *,
                            	 ggpointer,
                            	 gpointer);
void GtkWidget::size-allocate	(GtkWidget *,
                             	 ggpointer,
                             	 gpointer);
void GtkWidget::state-changed	(GtkWidget *,
                             	 GtkStateType,
                             	 gpointer);
void GtkWidget::parent-set	(GtkWidget *,
                          	 GtkObject *,
                          	 gpointer);
void GtkWidget::style-set	(GtkWidget *,
                         	 GtkStyle *,
                         	 gpointer);
void GtkWidget::add-accelerator	(GtkWidget *,
                               	 gguint,
                               	 GtkAccelGroup *,
                               	 gguint,
                               	 GdkModifierType,
                               	 GtkAccelFlags,
                               	 gpointer);
void GtkWidget::remove-accelerator	(GtkWidget *,
                                  	 GtkAccelGroup *,
                                  	 gguint,
                                  	 GdkModifierType,
                                  	 gpointer);
bool GtkWidget::event	(GtkWidget *,
                         	 GdkEvent *,
                         	 gpointer);
bool GtkWidget::button-press-event	(GtkWidget *,
                                      	 GdkEvent *,
                                      	 gpointer);
bool GtkWidget::button-release-event	(GtkWidget *,
                                        	 GdkEvent *,
                                        	 gpointer);
bool GtkWidget::motion-notify-event	(GtkWidget *,
                                       	 GdkEvent *,
                                       	 gpointer);
bool GtkWidget::delete-event	(GtkWidget *,
                                	 GdkEvent *,
                                	 gpointer);
bool GtkWidget::destroy-event	(GtkWidget *,
                                 	 GdkEvent *,
                                 	 gpointer);
bool GtkWidget::expose-event	(GtkWidget *,
                                	 GdkEvent *,
                                	 gpointer);
bool GtkWidget::key-press-event	(GtkWidget *,
                                   	 GdkEvent *,
                                   	 gpointer);
bool GtkWidget::key-release-event	(GtkWidget *,
                                     	 GdkEvent *,
                                     	 gpointer);
bool GtkWidget::enter-notify-event	(GtkWidget *,
                                      	 GdkEvent *,
                                      	 gpointer);
bool GtkWidget::leave-notify-event	(GtkWidget *,
                                      	 GdkEvent *,
                                      	 gpointer);
bool GtkWidget::configure-event	(GtkWidget *,
                                   	 GdkEvent *,
                                   	 gpointer);
bool GtkWidget::focus-in-event	(GtkWidget *,
                                  	 GdkEvent *,
                                  	 gpointer);
bool GtkWidget::focus-out-event	(GtkWidget *,
                                   	 GdkEvent *,
                                   	 gpointer);
bool GtkWidget::map-event	(GtkWidget *,
                             	 GdkEvent *,
                             	 gpointer);
bool GtkWidget::unmap-event	(GtkWidget *,
                               	 GdkEvent *,
                               	 gpointer);
bool GtkWidget::property-notify-event	(GtkWidget *,
                                         	 GdkEvent *,
                                         	 gpointer);
bool GtkWidget::selection-clear-event	(GtkWidget *,
                                         	 GdkEvent *,
                                         	 gpointer);
bool GtkWidget::selection-request-event	(GtkWidget *,
                                           	 GdkEvent *,
                                           	 gpointer);
bool GtkWidget::selection-notify-event	(GtkWidget *,
                                          	 GdkEvent *,
                                          	 gpointer);
void GtkWidget::selection-get	(GtkWidget *,
                             	 GtkSelectionData *,
                             	 gguint,
                             	 gpointer);
void GtkWidget::selection-received	(GtkWidget *,
                                  	 GtkSelectionData *,
                                  	 gguint,
                                  	 gpointer);
bool GtkWidget::proximity-in-event	(GtkWidget *,
                                      	 GdkEvent *,
                                      	 gpointer);
bool GtkWidget::proximity-out-event	(GtkWidget *,
                                       	 GdkEvent *,
                                       	 gpointer);
void GtkWidget::drag-begin	(GtkWidget *,
                          	 GdkDragContext *,
                          	 gpointer);
void GtkWidget::drag-end	(GtkWidget *,
                        	 GdkDragContext *,
                        	 gpointer);
void GtkWidget::drag-data-delete	(GtkWidget *,
                                	 GdkDragContext *,
                                	 gpointer);
void GtkWidget::drag-leave	(GtkWidget *,
                          	 GdkDragContext *,
                          	 gguint,
                          	 gpointer);
bool GtkWidget::drag-motion	(GtkWidget *,
                               	 GdkDragContext *,
                               	 ggint,
                               	 ggint,
                               	 gguint,
                               	 gpointer);
bool GtkWidget::drag-drop	(GtkWidget *,
                             	 GdkDragContext *,
                             	 ggint,
                             	 ggint,
                             	 gguint,
                             	 gpointer);
void GtkWidget::drag-data-get	(GtkWidget *,
                             	 GdkDragContext *,
                             	 GtkSelectionData *,
                             	 gguint,
                             	 gguint,
                             	 gpointer);
void GtkWidget::drag-data-received	(GtkWidget *,
                                  	 GdkDragContext *,
                                  	 ggint,
                                  	 ggint,
                                  	 GtkSelectionData *,
                                  	 gguint,
                                  	 gguint,
                                  	 gpointer);
bool GtkWidget::client-event	(GtkWidget *,
                                	 GdkEvent *,
                                	 gpointer);
bool GtkWidget::no-expose-event	(GtkWidget *,
                                   	 GdkEvent *,
                                   	 gpointer);
bool GtkWidget::visibility-notify-event	(GtkWidget *,
                                           	 GdkEvent *,
                                           	 gpointer);
void GtkWidget::debug-msg	(GtkWidget *,
                         	 GtkString *,
                         	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkData
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkData::disconnect	(GtkData *,
                        	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkContainer
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkContainer::add	(GtkContainer *,
                      	 GtkWidget *,
                      	 gpointer);
void GtkContainer::remove	(GtkContainer *,
                         	 GtkWidget *,
                         	 gpointer);
void GtkContainer::check-resize	(GtkContainer *,
                               	 gpointer);
GtkDirectionType GtkContainer::focus	(GtkContainer *,
                                    	 GtkDirectionType,
                                    	 gpointer);
void GtkContainer::set-focus-child	(GtkContainer *,
                                  	 GtkWidget *,
                                  	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkCalendar
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkCalendar::month-changed	(GtkCalendar *,
                               	 gpointer);
void GtkCalendar::day-selected	(GtkCalendar *,
                              	 gpointer);
void GtkCalendar::day-selected-double-click	(GtkCalendar *,
                                           	 gpointer);
void GtkCalendar::prev-month	(GtkCalendar *,
                            	 gpointer);
void GtkCalendar::next-month	(GtkCalendar *,
                            	 gpointer);
void GtkCalendar::prev-year	(GtkCalendar *,
                           	 gpointer);
void GtkCalendar::next-year	(GtkCalendar *,
                           	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkEditable
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkEditable::changed	(GtkEditable *,
                         	 gpointer);
void GtkEditable::insert-text	(GtkEditable *,
                             	 GtkString *,
                             	 ggint,
                             	 ggpointer,
                             	 gpointer);
void GtkEditable::delete-text	(GtkEditable *,
                             	 ggint,
                             	 ggint,
                             	 gpointer);
void GtkEditable::activate	(GtkEditable *,
                          	 gpointer);
void GtkEditable::set-editable	(GtkEditable *,
                              	 bool,
                              	 gpointer);
void GtkEditable::move-cursor	(GtkEditable *,
                             	 ggint,
                             	 ggint,
                             	 gpointer);
void GtkEditable::move-word	(GtkEditable *,
                           	 ggint,
                           	 gpointer);
void GtkEditable::move-page	(GtkEditable *,
                           	 ggint,
                           	 ggint,
                           	 gpointer);
void GtkEditable::move-to-row	(GtkEditable *,
                             	 ggint,
                             	 gpointer);
void GtkEditable::move-to-column	(GtkEditable *,
                                	 ggint,
                                	 gpointer);
void GtkEditable::kill-char	(GtkEditable *,
                           	 ggint,
                           	 gpointer);
void GtkEditable::kill-word	(GtkEditable *,
                           	 ggint,
                           	 gpointer);
void GtkEditable::kill-line	(GtkEditable *,
                           	 ggint,
                           	 gpointer);
void GtkEditable::cut-clipboard	(GtkEditable *,
                               	 gpointer);
void GtkEditable::copy-clipboard	(GtkEditable *,
                                	 gpointer);
void GtkEditable::paste-clipboard	(GtkEditable *,
                                 	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkTipsQuery
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkTipsQuery::start-query	(GtkTipsQuery *,
                              	 gpointer);
void GtkTipsQuery::stop-query	(GtkTipsQuery *,
                             	 gpointer);
void GtkTipsQuery::widget-entered	(GtkTipsQuery *,
                                 	 GtkWidget *,
                                 	 GtkString *,
                                 	 GtkString *,
                                 	 gpointer);
bool GtkTipsQuery::widget-selected	(GtkTipsQuery *,
                                      	 GtkWidget *,
                                      	 GtkString *,
                                      	 GtkString *,
                                      	 GdkEvent *,
                                      	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkCList
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkCList::select-row	(GtkCList *,
                         	 ggint,
                         	 ggint,
                         	 GdkEvent *,
                         	 gpointer);
void GtkCList::unselect-row	(GtkCList *,
                           	 ggint,
                           	 ggint,
                           	 GdkEvent *,
                           	 gpointer);
void GtkCList::row-move	(GtkCList *,
                       	 ggint,
                       	 ggint,
                       	 gpointer);
void GtkCList::click-column	(GtkCList *,
                           	 ggint,
                           	 gpointer);
void GtkCList::resize-column	(GtkCList *,
                            	 ggint,
                            	 ggint,
                            	 gpointer);
void GtkCList::toggle-focus-row	(GtkCList *,
                               	 gpointer);
void GtkCList::select-all	(GtkCList *,
                         	 gpointer);
void GtkCList::unselect-all	(GtkCList *,
                           	 gpointer);
void GtkCList::undo-selection	(GtkCList *,
                             	 gpointer);
void GtkCList::start-selection	(GtkCList *,
                              	 gpointer);
void GtkCList::end-selection	(GtkCList *,
                            	 gpointer);
void GtkCList::toggle-add-mode	(GtkCList *,
                              	 gpointer);
void GtkCList::extend-selection	(GtkCList *,
                               	 GtkScrollType,
                               	 ggfloat,
                               	 bool,
                               	 gpointer);
void GtkCList::scroll-vertical	(GtkCList *,
                              	 GtkScrollType,
                              	 ggfloat,
                              	 gpointer);
void GtkCList::scroll-horizontal	(GtkCList *,
                                	 GtkScrollType,
                                	 ggfloat,
                                	 gpointer);
void GtkCList::abort-column-resize	(GtkCList *,
                                  	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkNotebook
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkNotebook::switch-page	(GtkNotebook *,
                             	 ggpointer,
                             	 gguint,
                             	 gpointer);

</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>Gtk::List
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void Gtk::List::selection-changed	(Gtk::List *,
                               	 gpointer);
void Gtk::List::select-child	(Gtk::List *,
                          	 GtkWidget *,
                          	 gpointer);
void Gtk::List::unselect-child	(Gtk::List *,
                            	 GtkWidget *,
                            	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkMenuShell
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkMenuShell::deactivate	(GtkMenuShell *,
                             	 gpointer);
void GtkMenuShell::selection-done	(GtkMenuShell *,
                                 	 gpointer);
void GtkMenuShell::move-current	(GtkMenuShell *,
                               	 GtkMenuDirectionType,
                               	 gpointer);
void GtkMenuShell::activate-current	(GtkMenuShell *,
                                   	 bool,
                                   	 gpointer);
void GtkMenuShell::cancel	(GtkMenuShell *,
                         	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkToolbar
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkToolbar::orientation-changed	(GtkToolbar *,
                                    	 ggint,
                                    	 gpointer);
void GtkToolbar::style-changed	(GtkToolbar *,
                              	 ggint,
                              	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>Gtk::Tree
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void Gtk::Tree::selection-changed	(Gtk::Tree *,
                               	 gpointer);
void Gtk::Tree::select-child	(Gtk::Tree *,
                          	 GtkWidget *,
                          	 gpointer);
void Gtk::Tree::unselect-child	(Gtk::Tree *,
                            	 GtkWidget *,
                            	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkButton
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkButton::pressed	(GtkButton *,
                       	 gpointer);
void GtkButton::released	(GtkButton *,
                        	 gpointer);
void GtkButton::clicked	(GtkButton *,
                       	 gpointer);
void GtkButton::enter	(GtkButton *,
                     	 gpointer);
void GtkButton::leave	(GtkButton *,
                     	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkItem
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkItem::select	(GtkItem *,
                    	 gpointer);
void GtkItem::deselect	(GtkItem *,
                      	 gpointer);
void GtkItem::toggle	(GtkItem *,
                    	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkWindow
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkWindow::set-focus	(GtkWindow *,
                         	 ggpointer,
                         	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkHandleBox
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkHandleBox::child-attached	(GtkHandleBox *,
                                 	 GtkWidget *,
                                 	 gpointer);
void GtkHandleBox::child-detached	(GtkHandleBox *,
                                 	 GtkWidget *,
                                 	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkToggleButton
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkToggleButton::toggled	(GtkToggleButton *,
                             	 gpointer);

</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkMenuItem
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkMenuItem::activate	(GtkMenuItem *,
                          	 gpointer);
void GtkMenuItem::activate-item	(GtkMenuItem *,
                               	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>Gtk::ListItem
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void Gtk::ListItem::toggle-focus-row	(Gtk::ListItem *,
                                  	 gpointer);
void Gtk::ListItem::select-all	(Gtk::ListItem *,
                            	 gpointer);
void Gtk::ListItem::unselect-all	(Gtk::ListItem *,
                              	 gpointer);
void Gtk::ListItem::undo-selection	(Gtk::ListItem *,
                                	 gpointer);
void Gtk::ListItem::start-selection	(Gtk::ListItem *,
                                 	 gpointer);
void Gtk::ListItem::end-selection	(Gtk::ListItem *,
                               	 gpointer);
void Gtk::ListItem::toggle-add-mode	(Gtk::ListItem *,
                                 	 gpointer);
void Gtk::ListItem::extend-selection	(Gtk::ListItem *,
                                  	 GtkEnum,
                                  	 ggfloat,
                                  	 bool,
                                  	 gpointer);
void Gtk::ListItem::scroll-vertical	(Gtk::ListItem *,
                                 	 GtkEnum,
                                 	 ggfloat,
                                 	 gpointer);
void Gtk::ListItem::scroll-horizontal	(Gtk::ListItem *,
                                   	 GtkEnum,
                                   	 ggfloat,
                                   	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>Gtk::TreeItem
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void Gtk::TreeItem::collapse	(Gtk::TreeItem *,
                          	 gpointer);
void Gtk::TreeItem::expand	(Gtk::TreeItem *,
                        	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkCheckMenuItem
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkCheckMenuItem::toggled	(GtkCheckMenuItem *,
                              	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkInputDialog
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkInputDialog::enable-device	(GtkInputDialog *,
                                  	 ggint,
                                  	 gpointer);
void GtkInputDialog::disable-device	(GtkInputDialog *,
                                   	 ggint,
                                   	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkColorSelection
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkColorSelection::color-changed	(GtkColorSelection *,
                                     	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkStatusBar
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkStatusbar::text-pushed	(GtkStatusbar *,
                              	 gguint,
                              	 GtkString *,
                              	 gpointer);
void GtkStatusbar::text-popped	(GtkStatusbar *,
                              	 gguint,
                              	 GtkString *,
                              	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkCTree
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkCTree::tree-select-row	(GtkCTree *,
                              	 GtkCTreeNode *,
                              	 ggint,
                              	 gpointer);
void GtkCTree::tree-unselect-row	(GtkCTree *,
                                	 GtkCTreeNode *,
                                	 ggint,
                                	 gpointer);
void GtkCTree::tree-expand	(GtkCTree *,
                          	 GtkCTreeNode *,
                          	 gpointer);
void GtkCTree::tree-collapse	(GtkCTree *,
                            	 ggpointer,
                            	 gpointer);
void GtkCTree::tree-move	(GtkCTree *,
                        	 GtkCTreeNode *,
                        	 GtkCTreeNode *,
                        	 GtkCTreeNode *,
                        	 gpointer);
void GtkCTree::change-focus-row-expansion	(GtkCTree *,
                                         	 GtkCTreeExpansionType,
                                         	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkCurve
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkCurve::curve-type-changed	(GtkCurve *,
                                 	 gpointer);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>GtkAdjustment
<!-- ----------------------------------------------------------------- -->
<p>
<tscreen><verb>
void GtkAdjustment::changed	(GtkAdjustment *,
                           	 gpointer);
void GtkAdjustment::value-changed	(GtkAdjustment *,
                                 	 gpointer);
</verb></tscreen>

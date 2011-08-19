using System.Windows.Controls;

namespace AvalonEditUserControl
{
    public partial class AvalonEditTextEditorUC : UserControl
    {
        public AvalonEditTextEditorUC()
        {
            TextEditor = new ICSharpCode.AvalonEdit.TextEditor();
            InitializeComponent();
        }

        public ICSharpCode.AvalonEdit.TextEditor TextField
        {
            get
            {
                return TextEditor;
            }
        }
    }
}
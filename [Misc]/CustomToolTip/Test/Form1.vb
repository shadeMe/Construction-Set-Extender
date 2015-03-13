Imports System.Drawing.Drawing2D
Public Class Form1
    ' This event fired when the tooltip needs to draw its surface.
    ' Called when the OwnerDraw or OwnerDrawBackground property is set to true.
    Private Sub ToolTip1_Draw(ByVal sender As Object, ByVal e As Ai.Control.DrawEventArgs) Handles ToolTip1.Draw
        Dim strFormat As StringFormat = New StringFormat
        strFormat.LineAlignment = StringAlignment.Center
        strFormat.Alignment = StringAlignment.Center
        e.Graphics.DrawString("This is when OwnerDrawnBackground of the ToolTip is set to True", Me.Font, Brushes.Black, e.Rectangle, strFormat)
    End Sub
    ' This event fired when the tooltip needs to draw its background.
    ' Called only when OwnerDrawBackground property is set to true.
    Private Sub ToolTip1_DrawBackground(ByVal sender As Object, ByVal e As Ai.Control.DrawEventArgs) Handles ToolTip1.DrawBackground
        e.Graphics.SmoothingMode = SmoothingMode.AntiAlias
        e.Graphics.TextRenderingHint = Drawing.Text.TextRenderingHint.ClearTypeGridFit
        e.Graphics.FillEllipse(Brushes.White, e.Rectangle)
        e.Graphics.DrawEllipse(Pens.Black, e.Rectangle)
    End Sub
    ' This event fired before tooltip is displayed, mainly to provide the tooltip's size.
    ' Called when the OwnerDraw or OwnerDrawBackground property is set to true.
    Private Sub ToolTip1_Popup(ByVal sender As Object, ByVal e As Ai.Control.PopupEventArgs) Handles ToolTip1.Popup
        e.Size = New Size(100, 100)
    End Sub
End Class

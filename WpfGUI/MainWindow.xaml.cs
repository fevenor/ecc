using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace WpfGUI
{
    // <summary>
    // MainWindow.xaml 的交互逻辑
    // </summary>
    public partial class MainWindow : Window
    {
        internal Key newKey { get; private set; }

        public MainWindow()
        {
            InitializeComponent();
            getKeyButton.Click += new RoutedEventHandler(GetKey);
            savePrivateKeyButton.Click += new RoutedEventHandler(SavePrivateKey);
            savePublicKeyButton.Click += new RoutedEventHandler(SavePublicKey);
            
        }

        private void GetKey(object sender, RoutedEventArgs e)
        {
            newKey = new Key();
            newKey.GetKey(curveNameComboBox.Text);
            privateKeyTextBox.Text = newKey.privatekey.ToString();
            publicKeyTextBox.Text = newKey.publickey.ToString();
        }

        private void SavePrivateKey(object sender, RoutedEventArgs e)
        {
            SaveFileDialog saveFileDialog = new SaveFileDialog();
            saveFileDialog.Filter = "私钥文件|*.key";
            saveFileDialog.Title = "保存私钥";
            saveFileDialog.ShowDialog();
            newKey.SavePrivateKey(saveFileDialog.FileName);
        }

        void SavePublicKey(object sender, RoutedEventArgs e)
        {
            SaveFileDialog saveFileDialog = new SaveFileDialog();
            saveFileDialog.Filter = "公钥文件|*.pub";
            saveFileDialog.Title = "保存私钥";
            saveFileDialog.ShowDialog();
            newKey.SavePrivateKey(saveFileDialog.FileName);
        }
    }

    class Key
    {
        [DllImport("basefunc.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.Cdecl)]
        extern static int get_key([MarshalAs(UnmanagedType.LPStr)] string curve, [MarshalAs(UnmanagedType.LPStr)] StringBuilder privatekey, [MarshalAs(UnmanagedType.LPStr)] StringBuilder public_x, [MarshalAs(UnmanagedType.LPStr)] StringBuilder public_y);

        public String curvename { get; private set; }
        public StringBuilder privatekey { get; private set; }
        public StringBuilder publickey { get; private set; }
        public StringBuilder public_x { get; private set; }
        public StringBuilder public_y { get; private set; }

        public Key()
        {
            privatekey = new StringBuilder(65);
            publickey = new StringBuilder();
            public_x = new StringBuilder(65);
            public_y = new StringBuilder(65);
        }

        //根据曲线名生成密钥对
        public void GetKey(string curvename)
        {
            this.curvename = curvename;
            get_key(curvename, privatekey, public_x, public_y);
            publickey.Append(public_x).Append('\n').Append(public_y);
        }

        //保存私钥
        public void SavePrivateKey(string path)
        {
            File.WriteAllText(path, curvename + '\n' + privatekey.ToString());
        }

        //保存公钥
        public void SavePublicKey(string path)
        {
            File.WriteAllText(path, curvename + '\n' + publickey.ToString());
        }
    }
}

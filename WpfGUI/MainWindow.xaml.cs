﻿using Microsoft.Win32;
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
        public string statusInfo { get; private set; }

        public MainWindow()
        {
            InitializeComponent();

            getKeyButton.Click += new RoutedEventHandler(GetKey);
            savePrivateKeyButton.Click += new RoutedEventHandler(SavePrivateKey);
            savePublicKeyButton.Click += new RoutedEventHandler(SavePublicKey);

            getPublicKeyFilePathButton.Click += new RoutedEventHandler(GetPublicKeyFilePath);
            encryptionTypeComboBox.SelectionChanged += ChangeEncryptionInterface;
            encryptButton.Click += new RoutedEventHandler(Encrypt);
            getPlainFilePathButton.Click += new RoutedEventHandler(GetPlainFilePath);
            getCipherFilePathButton.Click += new RoutedEventHandler(GetCipherFilePath);

            getPrivateKeyFilePathButton.Click += new RoutedEventHandler(GetPrivateKeyFilePath);
            decryptionTypeComboBox.SelectionChanged += ChangeDecryptionInterface;
            decryptButton.Click += new RoutedEventHandler(Decrypt);
            getEncryptedFilePathButton.Click += new RoutedEventHandler(GetEncryptedFilePath);
            getDecryptedFilePathButton.Click += new RoutedEventHandler(GetDecryptedFilePath);
        }

        //密钥对生成的方法
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
        private void SavePublicKey(object sender, RoutedEventArgs e)
        {
            SaveFileDialog saveFileDialog = new SaveFileDialog();
            saveFileDialog.Filter = "公钥文件|*.pub";
            saveFileDialog.Title = "保存私钥";
            saveFileDialog.ShowDialog();
            newKey.SavePublicKey(saveFileDialog.FileName);
        }

        //加密的方法
        private void GetPublicKeyFilePath(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "公钥文件|*.pub";
            openFileDialog.Multiselect = false;
            openFileDialog.Title = "选择公钥文件";
            if (openFileDialog.ShowDialog() == true)
            {
                publicKeyFilePathTextBox.Text = openFileDialog.FileName;
            }
        }
        private void ChangeEncryptionInterface(object sender, SelectionChangedEventArgs e)
        {
            if (encryptionTypeComboBox.SelectedIndex == 0)
            {
                textEncryptionInterface.Visibility = Visibility.Visible;
                filetEncryptionInterface.Visibility = Visibility.Collapsed;
            }
            else if (encryptionTypeComboBox.SelectedIndex == 1)
            {
                textEncryptionInterface.Visibility = Visibility.Collapsed;
                filetEncryptionInterface.Visibility = Visibility.Visible;
            }
        }
        private void Encrypt(object sender, RoutedEventArgs e)
        {
            //加密时系统繁忙导致UI挂起，状态栏无法更新
            statusBar.Visibility = Visibility.Visible;
            statusBarTextBlock.Text = "读取文件...";
            Encryption encryption = new Encryption(File.ReadAllText(publicKeyFilePathTextBox.Text));
            byte[] plain = File.ReadAllBytes(plainFilePathTextBox.Text);
            statusBarTextBlock.Text = "加密文件...";
            byte[] cipher = encryption.Encrypt(plain);
            statusBarTextBlock.Text = "加密完成";
            System.Threading.Thread.Sleep(500);
            statusBarTextBlock.Text = "保存文件";
            File.WriteAllBytes(cipherFilePathTextBox.Text, cipher);
            statusBar.Visibility = Visibility.Collapsed;
            statusBarTextBlock.Text = "";

        }
        private void GetPlainFilePath(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Multiselect = false;
            openFileDialog.Title = "选择文件";
            if (openFileDialog.ShowDialog() == true)
            {
                plainFilePathTextBox.Text = openFileDialog.FileName;
                cipherFilePathTextBox.Text = openFileDialog.FileName + ".encrypted";
            }
        }
        private void GetCipherFilePath(object sender, RoutedEventArgs e)
        {
            SaveFileDialog saveFileDialog = new SaveFileDialog();
            saveFileDialog.Filter = "加密文件|*.encrypted";
            saveFileDialog.Title = "选择文件保存位置";
            if (cipherFilePathTextBox.Text != "")
            {
                saveFileDialog.InitialDirectory = System.IO.Path.GetDirectoryName(cipherFilePathTextBox.Text);
                saveFileDialog.FileName = System.IO.Path.GetFileName(cipherFilePathTextBox.Text);
            }
            if (saveFileDialog.ShowDialog() == true)
            {
                cipherFilePathTextBox.Text = saveFileDialog.FileName;
            }
        }

        //解密的方法
        private void GetPrivateKeyFilePath(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "私钥文件|*.key";
            openFileDialog.Multiselect = false;
            openFileDialog.Title = "选择私钥文件";
            if (openFileDialog.ShowDialog() == true)
            {
                privateKeyFilePathTextBox.Text = openFileDialog.FileName;
            }
        }
        private void ChangeDecryptionInterface(object sender, SelectionChangedEventArgs e)
        {
            if (decryptionTypeComboBox.SelectedIndex == 0)
            {
                textDecryptionInterface.Visibility = Visibility.Visible;
                filetDecryptionInterface.Visibility = Visibility.Collapsed;
            }
            else if (decryptionTypeComboBox.SelectedIndex == 1)
            {
                textDecryptionInterface.Visibility = Visibility.Collapsed;
                filetDecryptionInterface.Visibility = Visibility.Visible;
            }
        }
        private void Decrypt(object sender, RoutedEventArgs e)
        {
            statusBar.Visibility = Visibility.Visible;
            statusBarTextBlock.Text = "读取文件...";
            Decryption decryption = new Decryption(File.ReadAllText(privateKeyFilePathTextBox.Text));
            byte[] encrypted= File.ReadAllBytes(encryptedFilePathTextBox.Text);
            statusBarTextBlock.Text = "解密文件...";
            byte[] decrypted = decryption.Decrypt(encrypted);
            statusBarTextBlock.Text = "解密完成";
            System.Threading.Thread.Sleep(500);
            statusBarTextBlock.Text = "保存文件";
            File.WriteAllBytes(decryptedFilePathTextBox.Text, decrypted);
            statusBar.Visibility = Visibility.Collapsed;
            statusBarTextBlock.Text = "";
        }
        private void GetEncryptedFilePath(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "加密文件|*.encrypted";
            openFileDialog.Multiselect = false;
            openFileDialog.Title = "选择加密文件";
            if (openFileDialog.ShowDialog() == true)
            {
                encryptedFilePathTextBox.Text = openFileDialog.FileName;
                decryptedFilePathTextBox.Text = openFileDialog.FileName.Substring(0, openFileDialog.FileName.Length-10);
            }
        }
        private void GetDecryptedFilePath(object sender, RoutedEventArgs e)
        {
            SaveFileDialog saveFileDialog = new SaveFileDialog();
            saveFileDialog.Title = "选择文件保存位置";
            if (decryptedFilePathTextBox.Text != "")
            {
                saveFileDialog.InitialDirectory = System.IO.Path.GetDirectoryName(decryptedFilePathTextBox.Text);
                saveFileDialog.FileName = System.IO.Path.GetFileName(decryptedFilePathTextBox.Text);
            }
            if (saveFileDialog.ShowDialog() == true)
            {
                decryptedFilePathTextBox.Text = saveFileDialog.FileName;
            }
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

    class Encryption
    {
        [DllImport("basefunc.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.Cdecl)]
        extern static IntPtr encrypt([MarshalAs(UnmanagedType.LPStr)] string curve, [MarshalAs(UnmanagedType.LPStr)] string pub_x, [MarshalAs(UnmanagedType.LPStr)] string pub_y, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 4)] byte[] info, ulong info_length_byte, ref ulong cipherdata_length_byte);

        public string curve;
        public string pub_x;
        public string pub_y;

        public Encryption(string publickey)
        {
            string[] publickeys = publickey.Split('\n');

            curve = publickeys[0];
            pub_x = publickeys[1];
            pub_y = publickeys[2];
        }
        public Encryption(string curve, string pub_x, string pub_y)
        {
            this.curve = curve;
            this.pub_x = pub_x;
            this.pub_y = pub_y;
        }
        public byte[] Encrypt(byte[] plain)
        {
            ulong cipherdata_length_byte = 0;
            IntPtr temp = encrypt(curve, pub_x, pub_y, plain, (ulong)plain.Length, ref cipherdata_length_byte);
            byte[] cipher = new byte[cipherdata_length_byte];
            Marshal.Copy(temp, cipher, 0, (int)cipherdata_length_byte);
            return cipher;
        }
    }

    class Decryption
    {
        [DllImport("basefunc.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.Cdecl)]
        extern static IntPtr decrypt([MarshalAs(UnmanagedType.LPStr)] string key, [MarshalAs(UnmanagedType.LPArray)] byte[] secret, ulong cipherdata_length_byte, ref ulong plaindata_length_byte);

        public string privatekey;

        public Decryption(string privatekey)
        {
            this.privatekey = privatekey.Split('\n')[1];
        }

        public byte[] Decrypt(byte[] encrypted)
        {
            ulong decrypteddata_length_byte = 0;
            IntPtr temp = decrypt(privatekey, encrypted, (ulong)encrypted.Length, ref decrypteddata_length_byte);
            byte[] decrypted = new byte[decrypteddata_length_byte];
            Marshal.Copy(temp, decrypted, 0, (int)decrypteddata_length_byte);
            return decrypted;
        }
    }
}

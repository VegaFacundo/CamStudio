using System.Runtime.InteropServices;
using FFmpeg.AutoGen.Abstractions;
using System.Security.Cryptography;

namespace CamStudio.Services;

public unsafe class H264Decoder : IDisposable
{
    private AVCodecContext* _codecContext;
    private AVCodec* _codec;
    private AVPacket* _packet;
    private AVFrame* _frame;

    private SwsContext* _swsContext;

    private int _width;
    private int _height;

    public H264Decoder()
    {
        _codec = ffmpeg.avcodec_find_decoder(
            AVCodecID.AV_CODEC_ID_H264
        );

        if (_codec == null)
            throw new Exception(
                "H264 decoder no encontrado."
            );

        _codecContext =
            ffmpeg.avcodec_alloc_context3(_codec);

        if (_codecContext == null)
            throw new Exception(
                "No se pudo crear AVCodecContext."
            );
        _codecContext->time_base = new AVRational
        {
            num = 1,
            den = 1_000_000
        };
        int result = ffmpeg.avcodec_open2(
            _codecContext,
            _codec,
            null
        );

        if (result < 0)
            throw new Exception(
                $"No se pudo abrir H264 decoder. Error: {result}"
            );

        _packet = ffmpeg.av_packet_alloc();
        _frame = ffmpeg.av_frame_alloc();

        if (_packet == null || _frame == null)
            throw new Exception(
                "No se pudieron crear estructuras FFmpeg."
            );
    }

   

    public void Decode(
        byte[] h264Data,
        long pts,
        int flags,
        Action<byte[]> onFrame
    )
    {
        if (h264Data.Length == 0)
            return;
        

        PrintNalTypes(h264Data);
        string hash = Convert.ToHexString(
    SHA256.HashData(h264Data)
);

        
        SendPacket(
            h264Data,
            pts,
            flags,
            onFrame
        );
    }
    private void PrintNalTypes(byte[] data)
    {
        for (int i = 0; i < data.Length - 3; i++)
        {
            int startCodeLength = 0;

            if (
                i + 4 <= data.Length &&
                data[i] == 0 &&
                data[i + 1] == 0 &&
                data[i + 2] == 0 &&
                data[i + 3] == 1
            )
            {
                startCodeLength = 4;
            }
            else if (
                i + 3 <= data.Length &&
                data[i] == 0 &&
                data[i + 1] == 0 &&
                data[i + 2] == 1
            )
            {
                startCodeLength = 3;
            }

            if (startCodeLength > 0 &&
                i + startCodeLength < data.Length)
            {
                int nalType =
                    data[i + startCodeLength] & 0x1F;

                i += startCodeLength - 1;
            }
        }
    }
    private void SendPacket(
    byte[] data,
    long pts,
    int flags,
    Action<byte[]> onFrame
)
    {
        if (_packet == null || _codecContext == null)
            return;

        ffmpeg.av_packet_unref(_packet);

        int result = ffmpeg.av_new_packet(
            _packet,
            data.Length
        );

        if (result < 0)
        {

            return;
        }

        Marshal.Copy(
            data,
            0,
            (IntPtr)_packet->data,
            data.Length
        );

        _packet->pts = ffmpeg.AV_NOPTS_VALUE;
        _packet->dts = ffmpeg.AV_NOPTS_VALUE;


        result = ffmpeg.avcodec_send_packet(
            _codecContext,
            _packet
        );

        if (result < 0)
        {
          
            return;
        }

        ReceiveFrames(onFrame);
    }

    private void ReceiveFrames(
        Action<byte[]> onFrame
    )
    {
        while (true)
        {
            int result =
                ffmpeg.avcodec_receive_frame(
                    _codecContext,
                    _frame
                );

            if (
                result == ffmpeg.AVERROR(ffmpeg.EAGAIN) ||
                result == ffmpeg.AVERROR_EOF
            )
            {
                break;
            }

            if (result < 0)
            {
                 

                break;
            }
           
            byte[] bgrFrame =
                ConvertToBgr24();
           
            onFrame(bgrFrame);
        }
    }

    private byte[] ConvertToBgr24()
    {
        int width = _frame->width;
        int height = _frame->height;

        if (width <= 0 || height <= 0)
            throw new Exception(
                "Frame H264 inválido."
            );

        if (
            _swsContext == null ||
            width != _width ||
            height != _height
        )
        {
            if (_swsContext != null)
            {
                ffmpeg.sws_freeContext(
                    _swsContext
                );

                _swsContext = null;
            }

            _swsContext =
                ffmpeg.sws_getContext(
                    width,
                    height,
                    (AVPixelFormat)_frame->format,

                    width,
                    height,
                    AVPixelFormat.AV_PIX_FMT_BGR24,

                    2, // SWS_BILINEAR

                    null,
                    null,
                    null
                );

            if (_swsContext == null)
                throw new Exception(
                    "No se pudo crear SwsContext."
                );

            _width = width;
            _height = height;
        }

        int stride = width * 3;

        byte[] output =
            new byte[
                stride * height
            ];

        fixed (byte* outputPtr = output)
        {
            byte*[] dstData =
            {
                outputPtr,
                null,
                null,
                null
            };

            int[] dstLinesize =
            {
                stride,
                0,
                0,
                0
            };

            ffmpeg.sws_scale(
                _swsContext,

                _frame->data,
                _frame->linesize,

                0,
                height,

                dstData,
                dstLinesize
            );
        }

        return output;
    }

    public int Width => _width;

    public int Height => _height;

    public void Dispose()
    {
        if (_swsContext != null)
        {
            ffmpeg.sws_freeContext(
                _swsContext
            );

            _swsContext = null;
        }

        if (_frame != null)
        {
            AVFrame* frame = _frame;

            ffmpeg.av_frame_free(
                &frame
            );

            _frame = null;
        }

        if (_packet != null)
        {
            AVPacket* packet = _packet;

            ffmpeg.av_packet_free(
                &packet
            );

            _packet = null;
        }

        if (_codecContext != null)
        {
            AVCodecContext* context =
                _codecContext;

            ffmpeg.avcodec_free_context(
                &context
            );

            _codecContext = null;
        }

        _codec = null;
    }
}
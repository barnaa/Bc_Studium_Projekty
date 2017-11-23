% Vypracoval Andrej Barna (xbarna01@stud.fit.vutbr.cz), FIT VUT 2015/2016
iptsetpref('UseIPPL',false);

% Nazov upravovaneho obrazku, ocakavame spustanie tohto kodu v zlozke, kde sa
% maju nachadzat vysledky!
srcimg = imread('xbarna01.bmp');
srcimgsize = size(srcimg);

% Linearny Filter
linfil = [-0.5 -0.5 -0.5; -0.5 5 -0.5; -0.5 -0.5 -0.5];
step1 = imfilter(srcimg, linfil);
imwrite(step1, 'step1.bmp');

% Preklopenie obrazka
step2 = fliplr(step1);
imwrite(step2, 'step2.bmp');

% Medianovy filter, medfilt2 berie defaultne 3x3 oblast
step3 = medfilt2(step2, [5 5]);
imwrite(step3, 'step3.bmp');

% Rozmazanie obrazu
blurfil = ones(5);
blurfil(2:4, 2:4) = [3 3 3; 3 9 3; 3 3 3];
% nebudem to zbytocne takto natahovat
%blurfil(2:4, 2:4) = blurfil(2:4, 2:4)*3;
%blurfil(3,3) = 9;
blurfil = blurfil / 49;
step4 = imfilter(step3, blurfil);
imwrite(step4, 'step4.bmp');

% Vypocet chyby
s4flip = fliplr(step4);
srcimgdbl = double(srcimg);
s4flipdbl = double(s4flip);
dif = sum(sum(imabsdiff(srcimgdbl, s4flipdbl))) / (srcimgsize(1) * srcimgsize(2))

% Histogram
s4dbl = im2double(step4);
s4min = min(min(s4dbl));
s4max = max(max(s4dbl));
s4hist = imadjust(s4dbl, [s4min; s4max], [0.0; 1.0]);
step5 = im2uint8(s4hist);
imwrite(step5, 'step5.bmp');

% Vypocet strednej hodnoty a smerodatnej odchylky
mean_no_hist = mean2(step4)
std_no_hist = std2(step4)
mean_hist = mean2(step5)
std_hist = std2(step5)

% Kvantizacia
s5dbl = double(step5);
for x = 1:srcimgsize(1)
  for y = 1:srcimgsize(2)
  	% X je bod obrazu, a je 0, b je 255 a N je 2
    %   round(((2^N)-1)*(double(I5)-a)/(b-a))*(b-a)/((2^N)-1) + a;
    %step6(x,y) = round(((2^N)-1)*(s5dbl(x,y)-a)/(b-a))*(b-a)/((2^N)-1) + a;
    step6(x,y) = round(3*s5dbl(x,y)/255)*255/3;
  end
end
step6 = uint8(step6);
imwrite(step6, 'step6.bmp');
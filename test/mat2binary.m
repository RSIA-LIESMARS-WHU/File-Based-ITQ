TOTAL_SIZE = 2047379;
DIMENSIONS = 32;
BATCH_SIZE = 1000000; % set each binary file contain how many feature vectors

feats = randn(TOTAL_SIZE, DIMENSIONS);
feats = 2 * (feats - min(min(feats))) / (max(max(feats)) - min(min(feats)));

% zero-centered the data, IT IS VERY IMPORTANT!
feats = feats - repmat(mean(feats), size(feats, 1), 1);
feats = feats';

BIN_FILE_NUM = ceil(TOTAL_SIZE / BATCH_SIZE);

% mat to binary file
mkdir('dataset');
for i = 1:BIN_FILE_NUM
    fid = fopen(['dataset/data_' num2str(i - 1) '.bin'], 'wb');
    left = BATCH_SIZE * (i - 1) + 1;
    right = min(BATCH_SIZE * i, TOTAL_SIZE);
    fs = feats(:,left:right);
    fs = fs(:);
    fwrite(fid, fs, 'float');
    fclose(fid);
end
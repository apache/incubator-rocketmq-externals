package org.apache.rocketmq.flink.common.serialization.json;

import org.apache.flink.api.common.serialization.DeserializationSchema;
import org.apache.flink.configuration.ConfigOption;
import org.apache.flink.configuration.ConfigOptions;
import org.apache.flink.configuration.ReadableConfig;
import org.apache.flink.formats.json.JsonOptions;
import org.apache.flink.formats.json.TimestampFormat;
import org.apache.flink.table.api.ValidationException;
import org.apache.flink.table.connector.format.DecodingFormat;
import org.apache.flink.table.connector.format.EncodingFormat;
import org.apache.flink.table.data.RowData;
import org.apache.flink.table.factories.DeserializationFormatFactory;
import org.apache.flink.table.factories.DynamicTableFactory;
import org.apache.flink.table.factories.FactoryUtil;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

/**
 * @Author: gaobo07
 * @Date: 2020/10/7 2:45 下午
 */
public class McqJsonFormatFactory implements DeserializationFormatFactory, McqSerializationFormatFactory {

    public static final String IDENTIFIER = "mcq-json";

    public static final ConfigOption<Boolean> IGNORE_PARSE_ERRORS = JsonOptions.IGNORE_PARSE_ERRORS;

    public static final ConfigOption<String> TIMESTAMP_FORMAT = JsonOptions.TIMESTAMP_FORMAT;

    public static final ConfigOption<Integer> SINK_KEY_FIELD_POS = ConfigOptions
            .key("key.position")
            .intType()
            .noDefaultValue()
            .withDescription("Required Key field position in mcq producer");

    @Override
    public String factoryIdentifier() {
        return IDENTIFIER;
    }

    @Override
    public Set<ConfigOption<?>> requiredOptions() {
        return Collections.emptySet();
    }

    @Override
    public Set<ConfigOption<?>> optionalOptions() {
        Set<ConfigOption<?>> options = new HashSet<>();
        options.add(IGNORE_PARSE_ERRORS);
        options.add(TIMESTAMP_FORMAT);
        options.add(SINK_KEY_FIELD_POS);
        return options;
    }

    @Override
    public DecodingFormat<DeserializationSchema<RowData>> createDecodingFormat(DynamicTableFactory.Context context, ReadableConfig formatOptions) {
        // either implement your custom validation logic here ...
        // or use the provided helper method
        FactoryUtil.validateFactoryOptions(this, formatOptions);

        final boolean ignoreParseErrors = formatOptions.get(IGNORE_PARSE_ERRORS);
        TimestampFormat timestampFormatOption = JsonOptions.getTimestampFormat(formatOptions);

        // create and return the format
        return new McqJsonDecodingFormat(ignoreParseErrors, timestampFormatOption);
    }

    @Override
    public EncodingFormat<McqSerializationSchema<RowData>> createEncodingFormat(
            DynamicTableFactory.Context context,
            ReadableConfig formatOptions) {

        FactoryUtil.validateFactoryOptions(this, formatOptions);
        //when sink, rmq need key
        if(formatOptions.get(SINK_KEY_FIELD_POS) == null){
            throw new ValidationException(
                    String.format(
                            "One or more required options are missing.\n" +
                                    "Missing required options are:\n" +
                                    "%s", formatOptions.get(SINK_KEY_FIELD_POS)));
        }

        TimestampFormat timestampFormat = JsonOptions.getTimestampFormat(formatOptions);

        return new McqJsonEncodeingFormat(timestampFormat, formatOptions.get(SINK_KEY_FIELD_POS));
    }

}

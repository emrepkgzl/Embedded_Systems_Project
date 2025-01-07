package com.example.egemobil;

import static com.mapbox.maps.plugin.gestures.GesturesUtils.getGestures;
import static com.mapbox.maps.plugin.locationcomponent.LocationComponentUtils.getLocationComponent;
import android.Manifest;

import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.content.res.AppCompatResources;
import androidx.core.app.ActivityCompat;

import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import com.google.android.gms.maps.model.LatLng;
import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.mapbox.android.gestures.MoveDistancesObject;
import com.mapbox.android.gestures.MoveGestureDetector;
import com.mapbox.geojson.Geometry;
import com.mapbox.geojson.Point;
import com.mapbox.maps.CameraOptions;
import com.mapbox.maps.Image;
import com.mapbox.maps.ImageHolder;
import com.mapbox.maps.MapView;
import com.mapbox.maps.MapboxStyleManager;
import com.mapbox.maps.Style;
import com.mapbox.maps.StyleManager;
import com.mapbox.maps.extension.style.layers.LayerUtils;
import com.mapbox.maps.extension.style.layers.generated.CircleLayer;
import com.mapbox.maps.extension.style.layers.generated.SymbolLayer;
import com.mapbox.maps.extension.style.layers.properties.generated.SymbolPlacement;
import com.mapbox.maps.plugin.LocationPuck2D;
//import com.mapbox.maps.plugin.annotation.annotations.Symbol;
import com.mapbox.maps.plugin.MapPlugin;
import com.mapbox.maps.plugin.annotation.*;
import com.mapbox.maps.plugin.annotation.AnnotationPlugin;;
import com.mapbox.maps.plugin.annotation.generated.CircleAnnotation;
import com.mapbox.maps.plugin.annotation.generated.CircleAnnotationManager;
import com.mapbox.maps.plugin.annotation.generated.CircleAnnotationManagerKt;
import com.mapbox.maps.plugin.annotation.generated.CircleAnnotationOptions;
import com.mapbox.maps.plugin.annotation.generated.PointAnnotationManager;
import com.mapbox.maps.plugin.delegates.MapAttributionDelegate;
import com.mapbox.maps.plugin.delegates.MapCameraManagerDelegate;
import com.mapbox.maps.plugin.delegates.MapDelegateProvider;
import com.mapbox.maps.plugin.delegates.MapFeatureQueryDelegate;
import com.mapbox.maps.plugin.delegates.MapListenerDelegate;
import com.mapbox.maps.plugin.delegates.MapPluginProviderDelegate;
import com.mapbox.maps.plugin.delegates.MapProjectionDelegate;
import com.mapbox.maps.plugin.delegates.MapTransformDelegate;
import com.mapbox.maps.plugin.gestures.OnMoveListener;
import com.mapbox.maps.plugin.locationcomponent.LocationComponentPlugin;
import com.mapbox.maps.plugin.locationcomponent.LocationProvider;
import com.mapbox.maps.plugin.locationcomponent.OnIndicatorBearingChangedListener;
import com.mapbox.maps.plugin.locationcomponent.OnIndicatorPositionChangedListener;

import kotlin.Unit;
import kotlin.jvm.functions.Function1;


public class Map extends AppCompatActivity {

    MapView mapView;
    FloatingActionButton floatingActionButton;

    private final ActivityResultLauncher<String> activityResultLauncher = registerForActivityResult(new ActivityResultContracts.RequestPermission(), new ActivityResultCallback<Boolean>() {
        @Override
        public void onActivityResult(Boolean result) {
            if(result)
            {
                Toast.makeText(Map.this, "Permission Granted!", Toast.LENGTH_SHORT);
            }
            else {
                Toast.makeText(Map.this, "Permission Not Granted!!!", Toast.LENGTH_SHORT);
            }
        }
    });

    private final OnIndicatorBearingChangedListener onIndicatorBearingChangedListener = new OnIndicatorBearingChangedListener() {
        @Override
        public void onIndicatorBearingChanged(double v) {
            mapView.getMapboxMap().setCamera(new CameraOptions.Builder().bearing(v).build());
        }
    };

    private final OnIndicatorPositionChangedListener onIndicatorPositionChangedListener = new OnIndicatorPositionChangedListener() {
        @Override
        public void onIndicatorPositionChanged(@NonNull Point point) {
            mapView.getMapboxMap().setCamera(new CameraOptions.Builder().center(point).zoom(20.0).build());
            getGestures(mapView).setFocalPoint(mapView.getMapboxMap().pixelForCoordinate(point));
        }
    };

    private final OnMoveListener onMoveListener = new OnMoveListener() {
        @Override
        public void onMoveBegin(@NonNull MoveGestureDetector moveGestureDetector) {
            getLocationComponent(mapView).removeOnIndicatorPositionChangedListener(onIndicatorPositionChangedListener);
            getLocationComponent(mapView).removeOnIndicatorBearingChangedListener(onIndicatorBearingChangedListener);
            getGestures(mapView).removeOnMoveListener(onMoveListener);
            floatingActionButton.show();
        }

        @Override
        public boolean onMove(@NonNull MoveGestureDetector moveGestureDetector) {
            return false;
        }

        @Override
        public void onMoveEnd(@NonNull MoveGestureDetector moveGestureDetector) {

        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.activity_map);

        //mapView = findViewById(R.id.mapView);
        //floatingActionButton = findViewById(R.id.focusLocation);
        //floatingActionButton.hide();

        if(ActivityCompat.checkSelfPermission(Map.this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED)
        {
            activityResultLauncher.launch((Manifest.permission.ACCESS_FINE_LOCATION));
        }

        mapView.getMapboxMap().loadStyleUri(Style.MAPBOX_STREETS, new Style.OnStyleLoaded() {

            @Override
            public void onStyleLoaded(@NonNull Style style) {
                mapView.getMapboxMap().setCamera(new CameraOptions.Builder().zoom(20.0).build());
                LocationComponentPlugin locationComponentPlugin = getLocationComponent(mapView);
                locationComponentPlugin.setEnabled(true);
                //////////////////////////////////////DENEME

                //Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.gamepad);

                /*AnnotationPlugin annotationPlugin = AnnotationPluginImplKt.getAnnotations(mapView);
                CircleAnnotationManager circleAnnotationManager = CircleAnnotationManagerKt.createCircleAnnotationManager(annotationPlugin, new AnnotationConfig());
                CircleAnnotationOptions circleAnnotationOptions = new CircleAnnotationOptions()
                        // Define a geographic coordinate.
                        .withPoint(Point.fromLngLat(18.06, 59.31))
                        // Style the circle that will be added to the map.
                        .withCircleRadius(1.0)
                        .withCircleColor("#ee4e8b")
                        //.withCircleStrokeWidth(2.0)
                        .withCircleStrokeColor("#ffffff");*/


                //circleAnnotationManager.create(circleAnnotationOptions);

                //AnnotationPlugin annotationPlugin = AnnotationPluginImplKt.getAnnotations(mapView);
                //AnnotationPlugin

                //////////////////////////////////////DENEME
                LocationPuck2D locationPuck2D = new LocationPuck2D();
                ImageHolder imageHolder = ImageHolder.from(R.drawable.baseline_my_location_24);
                locationPuck2D.setBearingImage(imageHolder);
                locationComponentPlugin.addOnIndicatorBearingChangedListener(onIndicatorBearingChangedListener);
                getGestures(mapView).addOnMoveListener(onMoveListener);

                floatingActionButton.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        locationComponentPlugin.addOnIndicatorBearingChangedListener(onIndicatorBearingChangedListener);
                        locationComponentPlugin.addOnIndicatorPositionChangedListener(onIndicatorPositionChangedListener);
                        getGestures(mapView).addOnMoveListener(onMoveListener);
                        floatingActionButton.hide();
                    }
                });

            }
        });
    }
}